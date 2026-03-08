#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) return 0; 
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

// Function to execute bash and capture output
void execute_system_command(const char *cmd, char *output_buffer, size_t buffer_size) {
    printf("[SYSTEM] Intercepted Execution: %s\n", cmd);
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        snprintf(output_buffer, buffer_size, "Error: Failed to run command.");
        return;
    }
    
    output_buffer[0] = '\0'; // Clear buffer
    char line[256];
    while (fgets(line, sizeof(line), pipe) != NULL) {
        // Prevent buffer overflow
        if (strlen(output_buffer) + strlen(line) < buffer_size - 1) {
            strcat(output_buffer, line);
        }
    }
    pclose(pipe);
    printf("[SYSTEM] Output Captured (%zu bytes)\n", strlen(output_buffer));
}

// Function to send a UDP packet to the ESP32 Face
void send_to_esp32(const char *message) {
    int sockfd;
    struct sockaddr_in servaddr;

    // Create a UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return;

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(1234); // The port from your Arduino code
    servaddr.sin_addr.s_addr = inet_addr("10.42.0.95"); // Your ESP32's IP

    // Fire the message
    sendto(sockfd, message, strlen(message), MSG_CONFIRM, 
          (const struct sockaddr *)&servaddr, sizeof(servaddr));

    close(sockfd);
}

// Function to talk to Ollama
void ask_ollama(const char *system_prompt, const char *user_prompt, char *ai_response_buffer) {
    CURL *curl;
    CURLcode res;
    
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "model", "qwen2.5:0.5b");
    cJSON_AddStringToObject(root, "system", system_prompt);
    cJSON_AddStringToObject(root, "prompt", user_prompt);
    cJSON_AddBoolToObject(root, "stream", 0); 

    char *json_data = cJSON_PrintUnformatted(root);
    struct MemoryStruct chunk;
    chunk.memory = malloc(1);  
    chunk.size = 0;    

    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/generate");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);
        if(res == CURLE_OK) {
            cJSON *json = cJSON_Parse(chunk.memory);
            if (json != NULL) {
                cJSON *response = cJSON_GetObjectItemCaseSensitive(json, "response");
                if (cJSON_IsString(response) && (response->valuestring != NULL)) {
                    strcpy(ai_response_buffer, response->valuestring);
                }
                cJSON_Delete(json);
            }
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    free(chunk.memory);
    free(json_data);
    cJSON_Delete(root);
}

// Function to pipe text directly into the Piper TTS engine
void speak_text(const char *text) {
    printf("[SYSTEM] Routing audio to hardware...\n");
    
    // Open a write-pipe directly to the audio engine chain. 
    // We add 2>/dev/null and -q to hide the messy ALSA diagnostic text from your clean terminal.
    FILE *audio_pipe = popen("./piper --model en_US-lessac-medium.onnx --output_raw 2>/dev/null | aplay -r 22050 -f S16_LE -t raw -D plughw:2,0 -q 2>/dev/null", "w");
    
    if (audio_pipe) {
        // Dump the AI's text straight into the neural network
        fprintf(audio_pipe, "%s", text);
        pclose(audio_pipe);
    } else {
        printf("[SYSTEM] Error: Audio bridge failed to open.\n");
    }
}

int main(void) {
    char user_input[512];
    char ai_response[4096];
    char bash_output[2048];
    char secondary_prompt[4096];
    char esp_buffer[4100]; // Buffer to format strings for the ESP32

    // The master instruction set (UPDATED to enforce <MACRO> tags)
    const char *agent_rules = "You are Jarvis, a system assistant. If the user asks for system info (like time, date, memory, files), DO NOT guess. Output ONLY the Linux command needed to find the answer, wrapped perfectly in <CMD> and </CMD>. Example: <CMD> date </CMD>. If it is just a chat question, answer normally. CRITICAL RULE: For EVERY response, you MUST end your answer by summarizing the core point in 3 to 5 words, wrapped in <MACRO> and </MACRO> tags.";
    const char *summary_rules = "You are Jarvis. Summarize the following raw terminal output naturally for the user. Do not read raw numbers if not needed, be conversational. CRITICAL RULE: You MUST end your response by summarizing the core point in 3 to 5 words, wrapped in <MACRO> and </MACRO> tags for the physical display.";

    curl_global_init(CURL_GLOBAL_ALL);
    printf("==========================================\n");
    printf("   JARVIS AGENT PROTOCOL ONLINE\n");
    printf("==========================================\n\n");

    // Boot Sequence Visuals
    send_to_esp32("LED:GREEN");
    send_to_esp32("LCD:Systems Online.");

    while(1) {
        // STATE 1: IDLE / WAITING
        send_to_esp32("LED:GREEN");
        send_to_esp32("LCD:Awaiting Input");
        
        printf("Perlin> ");
        if (fgets(user_input, sizeof(user_input), stdin) == NULL) break;
        user_input[strcspn(user_input, "\n")] = 0;
        
        if (strcmp(user_input, "exit") == 0) {
            send_to_esp32("LED:RED");
            send_to_esp32("LCD:Shutting Down...");
            break;
        }
        if (strlen(user_input) == 0) continue;

        // STATE 2: THINKING
        send_to_esp32("LED:BLUE");
        send_to_esp32("ANIM:BUTTERFLY"); // Trigger the animation!
        
        ai_response[0] = '\0';
        ask_ollama(agent_rules, user_input, ai_response);

        // Scan for the <CMD> tag
        char *cmd_start = strstr(ai_response, "<CMD>");
        char *cmd_end = strstr(ai_response, "</CMD>");

        if (cmd_start && cmd_end) {
            cmd_start += 5; 
            size_t cmd_len = cmd_end - cmd_start;
            char extracted_cmd[256];
            strncpy(extracted_cmd, cmd_start, cmd_len);
            extracted_cmd[cmd_len] = '\0';

            while(extracted_cmd[0] == ' ') memmove(extracted_cmd, extracted_cmd+1, strlen(extracted_cmd));

            // STATE 3: EXECUTING SYSTEM COMMAND
            send_to_esp32("LED:RED");
            snprintf(esp_buffer, sizeof(esp_buffer), "LCD:Run: %s", extracted_cmd);
            send_to_esp32(esp_buffer);
            
            execute_system_command(extracted_cmd, bash_output, sizeof(bash_output));

            // STATE 4: SUMMARIZING OUTPUT
            send_to_esp32("LED:BLUE");
            send_to_esp32("ANIM:BUTTERFLY"); // Keep animating while summarizing
            
            snprintf(secondary_prompt, sizeof(secondary_prompt), "The user asked: '%s'. The system executed '%s' and returned:\n%s\nSummarize this for the user.", user_input, extracted_cmd, bash_output);
            ai_response[0] = '\0';
            ask_ollama(summary_rules, secondary_prompt, ai_response);
        }

        // STATE 5: RESPONDING & PARSING MACRO
        send_to_esp32("LED:GREEN");
        
        char macro_text[128];
        char clean_response[4096];
        
        // Default fallbacks in case the AI forgets the tag
        strcpy(macro_text, "Task Complete.");
        strcpy(clean_response, ai_response);

        // Scan for the <MACRO> tags
        char *macro_start = strstr(ai_response, "<MACRO>");
        char *macro_end = strstr(ai_response, "</MACRO>");

        if (macro_start && macro_end) {
            // Extract the crux for the LCD
            macro_start += 7; // Move past "<MACRO>"
            size_t macro_len = macro_end - macro_start;
            if (macro_len >= sizeof(macro_text)) macro_len = sizeof(macro_text) - 1;
            
            strncpy(macro_text, macro_start, macro_len);
            macro_text[macro_len] = '\0';

            // Slice the <MACRO> tag off the main response
            size_t clean_len = (macro_start - 7) - ai_response;
            strncpy(clean_response, ai_response, clean_len);
            clean_response[clean_len] = '\0';
        }

        // Send the crux to the physical LCD
        snprintf(esp_buffer, sizeof(esp_buffer), "LCD:%s", macro_text);
        send_to_esp32(esp_buffer); 

        // Print and Speak the clean response
        printf("JARVIS> %s\n\n", clean_response);
        speak_text(clean_response);
    }

    curl_global_cleanup();
    return 0;
}
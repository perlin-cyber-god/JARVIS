#include <stdio.h>
#include <stdlib.h>

int main() {
    char buffer[256];
    
    // 1. Imagine Ollama just generated this string:
    const char* ai_generated_command = "free -m"; 

    printf("[JARVIS ORCHESTRATOR] AI requested terminal access.\n");
    printf("[JARVIS ORCHESTRATOR] Executing: %s\n\n", ai_generated_command);

    // 2. popen() opens a pipe to the actual bash terminal
    // The "r" tells C we want to READ the terminal's output
    FILE* pipe = popen(ai_generated_command, "r");
    
    if (pipe == NULL) {
        printf("Failed to run command\n");
        return 1;
    }

    printf("--- CAPTURED BASH OUTPUT ---\n");
    
    // 3. Loop through the terminal output line-by-line and print it
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        printf("%s", buffer);
        // In the final version, instead of printing this, 
        // we will concatenate it into a giant string and send it back to Ollama!
    }
    
    printf("----------------------------\n");

    // 4. Safely close the terminal pipe
    pclose(pipe);
    
    return 0;
}
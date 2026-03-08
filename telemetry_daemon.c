#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// 1. Networking: Fire UDP to ESP32
void send_to_esp32(const char *message) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return;

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(1234);
    servaddr.sin_addr.s_addr = inet_addr("10.42.0.95");

    sendto(sockfd, message, strlen(message), MSG_CONFIRM, 
          (const struct sockaddr *)&servaddr, sizeof(servaddr));
    close(sockfd);
}

// 2. Hardware: Read Raw CPU Temperature
float get_cpu_temp() {
    FILE *f = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (!f) return 0.0;
    int temp;
    fscanf(f, "%d", &temp);
    fclose(f);
    return temp / 1000.0; // Kernel stores temp in millidegrees
}

// 3. Hardware: Read Raw RAM Usage
int get_ram_usage() {
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) return 0;
    char buffer[256];
    long total = 0, available = 0;
    
    while (fgets(buffer, sizeof(buffer), f)) {
        if (sscanf(buffer, "MemTotal: %ld kB", &total) == 1) continue;
        if (sscanf(buffer, "MemAvailable: %ld kB", &available) == 1) break;
    }
    fclose(f);
    
    if (total == 0) return 0;
    return (int)(((total - available) * 100.0) / total);
}

// 4. Hardware: Read Raw CPU Usage (Requires Delta Calculation)
unsigned long long prev_idle = 0, prev_total = 0;
int get_cpu_usage() {
    FILE *f = fopen("/proc/stat", "r");
    if (!f) return 0;
    char buffer[256];
    fgets(buffer, sizeof(buffer), f); // Read first line: "cpu ..."
    fclose(f);

    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    sscanf(buffer, "cpu %llu %llu %llu %llu %llu %llu %llu %llu", 
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);

    unsigned long long current_idle = idle + iowait;
    unsigned long long current_non_idle = user + nice + system + irq + softirq + steal;
    unsigned long long current_total = current_idle + current_non_idle;

    unsigned long long total_diff = current_total - prev_total;
    unsigned long long idle_diff = current_idle - prev_idle;

    prev_total = current_total;
    prev_idle = current_idle;

    if (total_diff == 0) return 0;
    return (int)(((total_diff - idle_diff) * 100.0) / total_diff);
}

// 5. Fan Controller (Placeholder for hardware wiring)
void set_fan_pwm(int duty_cycle) {
    // We will inject the hardware PWM code here in the next step!
    printf("[FAN CONTROLLER] Speed set to %d%%\n", duty_cycle);
}

int main(void) {
    printf("==========================================\n");
    printf("   TELEMETRY DAEMON ONLINE\n");
    printf("==========================================\n");

    char udp_buffer[256];
    
    // Initial CPU read to establish the baseline
    get_cpu_usage(); 
    sleep(1);

    while(1) {
        // Grab Vitals
        float temp = get_cpu_temp();
        int cpu = get_cpu_usage();
        int ram = get_ram_usage();

        // Print to Pi Terminal for debugging
        printf("Vitals -> Temp: %.1f°C | CPU: %d%% | RAM: %d%%\n", temp, cpu, ram);

        // ---------------------------------------------------------
        // FAN CURVE LOGIC
        // ---------------------------------------------------------
        int pwm = 0;
        if (temp < 40.0)      pwm = 20;
        else if (temp < 50.0) pwm = 50;
        else if (temp < 60.0) pwm = 80;
        else                  pwm = 100;
        
        set_fan_pwm(pwm);

        // ---------------------------------------------------------
        // EARLY WARNING ALERT SYSTEM (65°C / 80% / 80%)
        // ---------------------------------------------------------
        if (temp > 65.0 || cpu > 80 || ram > 80) {
            send_to_esp32("STATE:ALERT");
            printf("[!] ALERT THRESHOLD BREACHED!\n");
        } else {
            send_to_esp32("STATE:NOMINAL");
        }

        // ---------------------------------------------------------
        // UPDATE ESP32 LCD
        // ---------------------------------------------------------
        snprintf(udp_buffer, sizeof(udp_buffer), "UPDATE:%.1f,%d,%d", temp, cpu, ram);
        send_to_esp32(udp_buffer);

        // Run this loop every 2 seconds
        sleep(2);
    }
    return 0;
}
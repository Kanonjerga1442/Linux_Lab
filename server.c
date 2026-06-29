#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#define PORT 8080
#define KERNEL_PROC "/proc/sysmonitor"
#define LOG_FILE "system_monitor.log"

void write_log_low_level(const char *msg) {
    int fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd >= 0) { write(fd, msg, strlen(msg)); close(fd); }
}

void notify_kernel(const char *msg) {
    int fd = open(KERNEL_PROC, O_WRONLY);
    if (fd >= 0) { write(fd, msg, strlen(msg)); close(fd); }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[256] = {0};

    signal(SIGCHLD, SIG_IGN); // Chống Zombie process

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);
    
    printf("Server [Smart Mode] dang lang nghe tren port %d...\n", PORT);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        
        pid_t pid = fork();
        
        if (pid == 0) { 
            close(server_fd);
            read(new_socket, buffer, 256);
            
            int cpu;
            float ram;
            char sys_status[512];
            
            
            if (sscanf(buffer, "DATA|CPU:%d|RAM:%f", &cpu, &ram) == 2) {
                
                sprintf(sys_status, "[PID: %d] Thu thap -> CPU: %d%%, RAM: %.1f%%\n", getpid(), cpu, ram);
                write_log_low_level(sys_status); 
                
                
                if (ram > 80.0 || cpu > 90) {
                    char alert[512];
                    sprintf(alert, "[CRITICAL] CPU: %d%%, RAM: %.1f%% - Yeu cau can thiep!\n", cpu, ram);
                    
                    notify_kernel(alert); // Chỉ khi nguy hiểm mới làm phiền Kernel
                    printf("[!] PHAT HIEN NGUY HIEM - Đã báo Kernel!\n");
                    send(new_socket, "DANGER - Đã đẩy xuống Kernel!", 32, 0);
                } else {
                    printf("[i] Thông số an toàn.\n");
                    send(new_socket, "NORMAL - Đã ghi log", 21, 0);
                }
            } else {
                send(new_socket, "ERROR - Sai dinh dang du lieu", 30, 0);
            }
            
            close(new_socket);
            exit(0);
        } else if (pid > 0) {
            close(new_socket);
        }
    }
    return 0;
}

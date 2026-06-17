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

/* 1. FILE PROGRAMMING: Dùng System Call cấp thấp (open, write, close) */
void write_log_low_level(const char *msg) {
    // Mở file với cờ: Ghi thêm (APPEND), Tạo mới nếu chưa có (CREAT)
    int fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd >= 0) {
        write(fd, msg, strlen(msg));
        close(fd);
    }
}

void notify_kernel(const char *msg) {
    int fd = open(KERNEL_PROC, O_WRONLY);
    if (fd >= 0) {
        write(fd, msg, strlen(msg));
        close(fd);
    }
}

/* 2. PROCESS PROGRAMMING: Lấy thông tin Load Average của hệ điều hành */
void get_system_load(char *buffer) {
    int fd = open("/proc/loadavg", O_RDONLY);
    if (fd >= 0) {
        read(fd, buffer, 64);
        buffer[strcspn(buffer, "\n")] = 0; // Xóa ký tự xuống dòng
        close(fd);
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[256] = {0};

    // Bỏ qua tín hiệu kết thúc của tiến trình con để tránh Zombie Process
    signal(SIGCHLD, SIG_IGN);

    /* 3. SOCKET & NETWORK PROGRAMMING */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);
    
    printf("Server dang lang nghe tren port %d...\n", PORT);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        
        /* 4. PROCESS PROGRAMMING NÂNG CAO: Dùng fork() xử lý đa tiến trình */
        pid_t pid = fork();
        
        if (pid == 0) { 
            // ----- ĐÂY LÀ TIẾN TRÌNH CON (CHILD PROCESS) -----
            close(server_fd); // Tiến trình con không cần lắng nghe
            read(new_socket, buffer, 256);
            
            char load_avg[64] = {0};
            get_system_load(load_avg);
            
            char sys_status[512];
            sprintf(sys_status, "[PID: %d] Client gui: %s | OS Load Avg: %s\n", getpid(), buffer, load_avg);
            
            printf("%s", sys_status);
            
            // Ghi log (File Programming) & Đẩy xuống Kernel Module
            write_log_low_level(sys_status);
            notify_kernel(sys_status);
            
            send(new_socket, "Da tiep nhan", 12, 0);
            close(new_socket);
            exit(0); // Tiến trình con kết thúc
        } 
        else if (pid > 0) {
            // ----- ĐÂY LÀ TIẾN TRÌNH CHA (PARENT PROCESS) -----
            close(new_socket); // Cha tiếp tục vòng lặp chờ Client mới
        }
    }
    return 0;
}

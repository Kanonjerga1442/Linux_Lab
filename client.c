#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

/* Hàm này đọc trực tiếp file lõi của hệ điều hành để tính % RAM đang dùng */
float get_real_ram_usage() {
    FILE *file = fopen("/proc/meminfo", "r");
    if (!file) return -1.0;

    char line[256];
    long total_mem = 0, available_mem = 0;

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "MemTotal: %ld kB", &total_mem) == 1) continue;
        if (sscanf(line, "MemAvailable: %ld kB", &available_mem) == 1) break;
    }
    fclose(file);

    if (total_mem == 0) return 0.0;
    return ((float)(total_mem - available_mem) / total_mem) * 100.0;
}

int main() {
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    srand(time(NULL)); // Khởi tạo seed random cho CPU giả lập

    printf("Bat dau giam sat he thong (Gui du lieu 3s/lan)...\n");

    // Chạy vòng lặp vô hạn, đóng vai trò như một Agent chạy ngầm
    while (1) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("[!] Server dang dong, thu lai sau...\n");
            sleep(3);
            continue;
        }

        // 1. Thu thập dữ liệu
        float ram_usage = get_real_ram_usage();
        int fake_cpu = rand() % 100; // Random CPU từ 0-99%

        // 2. Đóng gói dữ liệu theo giao thức tự chế
        char message[256];
        sprintf(message, "DATA|CPU:%d|RAM:%.1f", fake_cpu, ram_usage);
        
        printf(" -> Gui: %s\n", message);
        send(sock, message, strlen(message), 0);
        
        // 3. Chờ phản hồi
        char buffer[256] = {0};
        read(sock, buffer, 256);
        printf(" <- Server: %s\n", buffer);
        
        close(sock);
        sleep(3); // Tạm nghỉ 3 giây trước khi đo và gửi lại
    }
    return 0;
}

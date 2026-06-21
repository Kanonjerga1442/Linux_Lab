# 🖥️ Ubuntu System Monitor (Advanced Linux Systems Programming)

Dự án này là một hệ thống giám sát và cảnh báo tài nguyên hệ thống (RAM/CPU) toàn diện trên môi trường Linux (Ubuntu). Hệ thống kết hợp các kỹ thuật lập trình hệ thống nâng cao nhằm thiết lập một luồng xử lý khép kín, tối ưu hiệu năng, bắc cầu an toàn giữa không gian người dùng (**User-space**) và không gian hạt nhân (**Kernel-space**).

---

##  1. Cấu trúc thư mục dự án (Directory Structure)

Dưới đây là sơ đồ tổ chức cấu trúc mã nguồn của hệ thống. Dự án được module hóa tách biệt rõ ràng giữa các thành phần logic:
```text
ubuntu_sysmonitor/
│
├──  Thành phần Tự động hóa (Bash Scripts)
│   ├── install.sh              # Kịch bản thiết lập hệ thống, đồng bộ thời gian và nạp module
│   ├── uninstall.sh            # Kịch bản dọn dẹp môi trường, giải phóng tài nguyên hệ thống
│   └── backup.sh               # Kịch bản sao lưu dữ liệu nhật ký, tự động đóng gói nén .gz
│
├──  Thành phần Không gian người dùng (User-space C Programs)
│   ├── client.c                # Ứng dụng Agent thu thập tài nguyên thực tế và truyền dữ liệu mạng
│   ├── server.c                # Ứng dụng Máy chủ mạng đa tiến trình, bóc tách chuỗi và ra quyết định
│   ├── client                  # Tệp chạy nhị phân sau khi biên dịch từ client.c (sinh ra khi cài đặt)
│   └── server                  # Tệp chạy nhị phân sau khi biên dịch từ server.c (sinh ra khi cài đặt)
│
├──  Thành phần Không gian hạt nhân (Linux Kernel-space)
│   ├── monitor_mod.c           # Mã nguồn C của Mô-đun nhân quản lý procfs và Workqueue (Deferred Work)
│   ├── Makefile                # Chỉ thị cấu hình cho trình biên dịch của nhân Linux (Kbuild)
│   ├── monitor_mod.ko          # Đối tượng nhị phân hạt nhân động (sinh ra khi biên dịch)
│   └── [monitor_mod.mod, ...]  # Các file bổ trợ tạm thời trong quá trình build kernel module
│
└──  Thành phần Dữ liệu & Nhật ký (Data & Logs)
    ├── system_monitor.log      # File log cục bộ lưu trữ toàn bộ dữ liệu do Server ghi nhận (System Call)
    └── /backup_logs/           # Thư mục hệ thống chứa các bản sao lưu đã nén (Màu đỏ .txt.gz)
 2. Luồng hoạt động tổng thể của hệ thống (System Architecture & Workflow)
Hệ thống vận hành theo một kiến trúc tuần hoàn, bất đồng bộ nhằm phân phối tải xử lý và đảm bảo an toàn cốt lõi cho hạt nhân hệ điều hành.

 Sơ đồ luồng dữ liệu (Dataflow Architecture)
Plaintext
==========================================================================================
                                    USER-SPACE (Không gian người dùng)
==========================================================================================
 [Client Agent]  Chạy vòng lặp 3s/lần
   │  ├── 1. Đọc file hệ thống ảo /proc/meminfo -> Tính toán % RAM thực tế sử dụng.
   │  └── 2. Giả lập thông số tải CPU (0% - 99%).
   │
   ▼ Đóng gói giao thức mạng theo định dạng chuỗi: "DATA|CPU:xx|RAM:yy"
 [Socket TCP Network (Port 8080)]
   │
   ▼ Giao tiếp mạng (Loopback IP: 127.0.0.1)
 [Server Monitor]  Sử dụng Đa tiến trình với fork()
   │
   ├── Tiến trình CHA: Giữ nhiệm vụ lắng nghe kết nối mới (Non-blocking listener).
   └── Tiến trình CON: Xử lý bóc tách (Parse) dữ liệu của luồng Client vừa kết nối.
         │
         ├── [Mọi tình huống] -> Ghi nhận thông số vào File hệ thống:
         │                      Sử dụng System Call cấp thấp (open(), write()) -> system_monitor.log
         │
         └── [Kiểm tra ngưỡng cảnh báo] -> Ra quyết định hành động:
               │
               ├── NẾU An toàn (RAM <= 80% và CPU <= 90%): 
               │   └── Trả phản hồi "NORMAL - Đã ghi log" về Client qua Network.
               │
               └── NẾU Vượt ngưỡng nguy hiểm (RAM > 80% hoặc CPU > 90%):
                   └── Trả phản hồi "DANGER" về Client qua Network.
                   │
                   ▼ Ghi chuỗi cảnh báo đặc biệt xuống Tệp ảo của hệ thống
====================== Biên giới Giao tiếp (User <-> Kernel Boundary) =====================
                      Lệnh ghi: write(fd, "[CRITICAL]...", ...) vào /proc/sysmonitor
==========================================================================================
                                   KERNEL-SPACE (Không gian hạt nhân)
==========================================================================================
 [File ảo /proc/sysmonitor]
   │
   ▼ Định hướng hành động bằng cấu trúc `proc_ops` (.proc_write -> procfile_write)
 [Hàm procfile_write của Module]
   │  ├── 1. Cấp phát bộ nhớ động an toàn trong nhân bằng kmalloc(..., GFP_KERNEL).
   │  ├── 2. Sao chép vùng nhớ an toàn từ User sang Kernel bằng copy_from_user().
   │  └── 3. Cơ chế DEFERRED WORK: Đóng gói dữ liệu vào cấu trúc chứa work_struct,
   │         gọi INIT_WORK() và queue_work() đẩy vào hàng đợi độc lập.
   │
   ▼ Giải phóng luồng xử lý ngay lập tức (Non-blocking write), nhường quyền cho hàng đợi ngầm
 [Hàng đợi công việc: monitor_workqueue]
   │
   ▼ Hệ điều hành sắp xếp lịch tự động chạy ngầm trên một Luồng Công nhân (Worker Thread)
 [Hàm xử lý trì hoãn: deferred_log_work]
      ├── 1. Trích xuất gói dữ liệu ngược lại bằng Macro container_of().
      ├── 2. Ghi nhật ký lõi bằng printk(KERN_INFO ...) hiển thị màu đỏ đặc trưng.
      └── 3. Giải phóng hoàn toàn vùng nhớ động bằng kfree() để chống rò rỉ RAM (Memory Leak).

==========================================================================================
                             TỰ ĐỘNG HÓA HỆ THỐNG (Cron Daemon & Bash)
==========================================================================================
 [Cronjob Daemon]  Đếm thời gian định kỳ (Mỗi phút một lần)
   │
   ▼ Kích hoạt thực thi ngầm
 [Script backup.sh]
      ├── 1. Kiểm tra sự tồn tại của file system_monitor.log.
      ├── 2. Sao chép và đánh dấu mốc thời gian: cp system_monitor.log /backup_logs/log_TIMESTAMP.txt
      └── 3. Nén nén tối ưu dung lượng đĩa: gzip /backup_logs/log_TIMESTAMP.txt (Tạo file đỏ .gz)
 3. Chi tiết chức năng từng File mã nguồn
1. install.sh (Bộ cài đặt & Đồng bộ)
Cấp quyền root ($EUID -ne 0).

Tự động hóa apt tải môi trường biên dịch hệ thống (gcc, make, linux-headers, cron).

Đồng bộ NTP và múi giờ hệ thống bằng lệnh timedatectl.

Tạo cấu hình Cronjob đẩy vào thư mục hệ thống /etc/cron.d/sys_backup.

Gọi lệnh biên dịch make (cho kernel module) và gcc (cho ứng dụng C), sau đó cấy nạp module vào lõi bằng insmod.

2. backup.sh (Quản lý File & Lập lịch)
Định nghĩa các hằng số đường dẫn tuyệt đối cho hệ thống.

Chụp ảnh dữ liệu log, đổi tên và áp dụng công cụ gzip đóng gói nén dữ liệu giúp tiết kiệm dung lượng đĩa và làm nổi bật màu nhận diện tệp nén trên Ubuntu Terminal.

3. uninstall.sh (Gỡ cài đặt tự động)
Tiến hành dọn dẹp hệ thống theo quy trình đảo ngược để tránh lỗi sập nhân (Kernel Panic).

Hủy cấu hình cron, xóa các tệp thực thi nhị phân, gọi rmmod đẩy module ra khỏi Kernel một cách an toàn và dọn sạch các tệp log tạm thời.

4. server.c (Lập trình Mạng, Tiến trình & File)
Network: Mở Socket, gán cổng (bind), lắng nghe (listen) kết nối TCP tại port 8080.

Process Management: Sử dụng hàm fork() để nhân bản tiến trình. Tiến trình con độc lập xử lý và bóc tách chuỗi bằng cú pháp sscanf(), sau đó thoát giải phóng RAM bằng exit(0).

Low-level File Control: Gọi các hàm hệ thống sơ khai open() với cờ tạo và ghi đè (O_WRONLY | O_CREAT | O_APPEND) kết hợp với lệnh write() thay vì dùng thư viện chuẩn fopen để đảm bảo tốc độ và tính nguyên khối của log.

5. client.c (Giám sát tài nguyên động)
Sử dụng hàm mở file để phân tích chuỗi văn bản trong file ảo hệ thống /proc/meminfo.

Trích xuất 2 trường MemTotal và MemAvailable, thực hiện phép toán: ((Total - Available) / Total) * 100 để tính ra số phần trăm bộ nhớ RAM thực tế đang bị chiếm dụng trên máy ảo Ubuntu tại thời điểm thực thi.

Giả lập tải CPU và thực hiện truyền tải không ngừng qua mạng nhờ vào vòng lặp trì hoãn thời gian sleep(3).

6. monitor_mod.c (Tích hợp hạt nhân & Hoãn việc)
Đăng ký cấu trúc hàm proc_ops với hệ điều hành Linux để tạo thực thể file ảo /proc/sysmonitor.

Nhúng thư viện <linux/workqueue.h> thiết lập cấu trúc hàng đợi bất đồng bộ. Đảm bảo nhân không bị quá tải khi xử lý dồn dập các cuộc gọi hệ thống từ các ứng dụng chạy ở môi trường ngoài.

 4. Hướng dẫn vận hành và Xem kết quả minh chứng
Bước 1: Khởi tạo toàn bộ hệ thống
sudo chmod +x *.sh
sudo ./install.sh

Bước 2: Chạy các chương trình giám sát
Mở Terminal 1, khởi chạy Server:
sudo ./server
Mở Terminal 2, khởi chạy Client Agent:
sudo ./client

Bước 3: Xem kết quả và Thu thập minh chứng
Kiểm tra Log không gian người dùng:
cat system_monitor.log

Kiểm tra Log không gian hạt nhân (Deferred Work dmesg):
sudo dmesg | grep "Monitor System"

Kiểm tra thành quả quản lý file và lập lịch nén của Cronjob:
ls -l /backup_logs/

Bước 4: Giải phóng và gỡ bỏ hoàn toàn
sudo ./uninstall.sh

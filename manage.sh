#!/bin/bash

# Kiểm tra quyền root
if [ "$EUID" -ne 0 ]; then
  echo "Vui long chay script voi quyen root (sudo ./manage.sh)"
  exit
fi

echo "=== Giao dien Quan ly He thong Ubuntu ==="

# 1. Cài đặt tự động các công cụ cần thiết
echo "[1] Cài đặt môi trường biên dịch (gcc, make, linux-headers)..."
apt-get update -qq
apt-get install -y gcc make linux-headers-$(uname -r) cron > /dev/null

# 2. Thiết lập thời gian hệ thống
echo "[2] Đồng bộ thời gian hệ thống với NTP..."
timedatectl set-ntp true
timedatectl set-timezone Asia/Ho_Chi_Minh

# 3. Lập lịch tự động bằng Cron (Sao lưu log mỗi phút để test, thực tế đổi thành 0 2 * * *)
echo "[3] Thiết lập Cronjob sao lưu log hệ thống..."
mkdir -p /backup_logs
CRON_JOB="* * * * * root cp $(pwd)/system_monitor.log /backup_logs/syslog_\$(date +\%Y\%m\%d_\%H\%M).log 2>/dev/null"
echo "$CRON_JOB" > /etc/cron.d/monitor_backup
chmod 644 /etc/cron.d/monitor_backup
systemctl restart cron

# 4. Biên dịch và nạp Kernel Module
echo "[4] Biên dịch và cài đặt Kernel Module..."
make clean
make
rmmod monitor_mod 2>/dev/null # Gỡ nếu đã tồn tại
insmod monitor_mod.ko

# 5. Biên dịch chương trình C
echo "[5] Biên dịch C Server và Client..."
gcc server.c -o server
gcc client.c -o client

echo "=== Hoàn tất thiết lập! ==="
echo "Các bước tiếp theo:"
echo "1. Chạy server: ./server"
echo "2. Mở terminal khác chạy client: ./client"
echo "3. Kiểm tra kernel log: dmesg | tail"

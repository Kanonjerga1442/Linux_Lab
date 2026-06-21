#!/bin/bash
if [ "$EUID" -ne 0 ]; then echo "Chạy bằng sudo!"; exit 1; fi

echo "=== 1. TIME SETTING ==="
timedatectl set-ntp true
timedatectl set-timezone Asia/Ho_Chi_Minh
echo "Đã đồng bộ thời gian: $(date)"

echo "=== 2. AUTO INSTALL KERNEL MODULE & C PROGRAM ==="
apt-get update -qq && apt-get install -y gcc make cron > /dev/null
make clean && make
insmod monitor_mod.ko
gcc server.c -o server
gcc client.c -o client
echo "Đã biên dịch và nạp Kernel Module."

echo "=== 3. CRON SCHEDULER ==="
mkdir -p /backup_logs
# Cài đặt cron chạy file backup.sh mỗi phút
SCRIPT_DIR=$(pwd)
echo "* * * * * root $SCRIPT_DIR/backup.sh" > /etc/cron.d/sys_backup
chmod 644 /etc/cron.d/sys_backup
systemctl restart cron
echo "Đã thiết lập Cronjob."


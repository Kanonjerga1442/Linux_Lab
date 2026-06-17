#!/bin/bash
if [ "$EUID" -ne 0 ]; then echo "Chạy bằng sudo!"; exit 1; fi

echo "=== AUTO REMOVE SYSTEM ==="
rm /etc/cron.d/sys_backup
systemctl restart cron
rmmod monitor_mod 2>/dev/null
make clean
rm server client system_monitor.log 2>/dev/null
echo "Đã gỡ Kernel Module, xóa Cronjob và dọn dẹp file biên dịch."

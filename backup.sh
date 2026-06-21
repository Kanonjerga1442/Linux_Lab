#!/bin/bash
# File này được gọi bởi Cronjob mỗi phút

LOG_FILE="/home/vinh/Desktop/ubuntu_sysmonitor/system_monitor.log"
BACKUP_DIR="/backup_logs"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

if [ -f "$LOG_FILE" ]; then
    # Copy và đổi tên file log
    cp "$LOG_FILE" "$BACKUP_DIR/log_$TIMESTAMP.txt"
    # Nén file log vừa copy thành định dạng .gz (Màu đỏ)
    gzip "$BACKUP_DIR/log_$TIMESTAMP.txt"
fi

#!/bin/bash
# File này được gọi bởi Cronjob

# Khai báo trực tiếp đường dẫn tuyệt đối
LOG_FILE="/home/vinh/Desktop/ubuntu_sysmonitor/system_monitor.log"
BACKUP_DIR="/backup_logs"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

if [ -f "$LOG_FILE" ]; then
    # Copy và đổi tên file log
    cp "$LOG_FILE" "$BACKUP_DIR/log_$TIMESTAMP.txt"
    # Nén file log (Shell quản lý file)
    gzip "$BACKUP_DIR/log_$TIMESTAMP.txt"
fi

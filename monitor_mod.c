#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

#define PROCFS_NAME "sysmonitor"
#define MSG_SIZE 256

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ubuntu Management System");
MODULE_DESCRIPTION("Kernel Module using Workqueue for System Monitoring");

static struct proc_dir_entry *our_proc_file;
static struct workqueue_struct *monitor_wq;

// Cấu trúc dữ liệu cho Workqueue
struct monitor_work {
    struct work_struct work;
    char message[MSG_SIZE];
};

// Hàm xử lý Deferred Work (chạy trong process context)
static void deferred_log_work(struct work_struct *work) {
    struct monitor_work *my_work = container_of(work, struct monitor_work, work);
    
    // Ghi log vào dmesg
    printk(KERN_INFO "[Monitor System - Workqueue] Nhận cảnh báo: %s", my_work->message);
    
    // Giải phóng bộ nhớ sau khi xử lý xong
    kfree(my_work);
}

// Hàm được gọi khi C Server (User-space) ghi vào /proc/sysmonitor
static ssize_t procfile_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos) {
    struct monitor_work *my_work;
    int len = count < MSG_SIZE - 1 ? count : MSG_SIZE - 1;

    // Cấp phát bộ nhớ động cho công việc mới
    my_work = kmalloc(sizeof(*my_work), GFP_KERNEL);
    if (!my_work) return -ENOMEM;

    if (copy_from_user(my_work->message, ubuf, len)) {
        kfree(my_work);
        return -EFAULT;
    }
    my_work->message[len] = '\0';

    // Khởi tạo công việc và đẩy vào hàng đợi (Queue the work)
    INIT_WORK(&my_work->work, deferred_log_work);
    queue_work(monitor_wq, &my_work->work);

    return count;
}

static const struct proc_ops proc_file_fops = {
    .proc_write = procfile_write,
};

static int __init monitor_init(void) {
    // 1. Khởi tạo proc file
    our_proc_file = proc_create(PROCFS_NAME, 0666, NULL, &proc_file_fops);
    if (!our_proc_file) {
        printk(KERN_ALERT "Loi: Khong the tao /proc/%s\n", PROCFS_NAME);
        return -ENOMEM;
    }

    // 2. Khởi tạo Workqueue riêng cho hệ thống
    monitor_wq = create_workqueue("monitor_workqueue");
    if (!monitor_wq) {
        proc_remove(our_proc_file);
        return -ENOMEM;
    }

    printk(KERN_INFO "[Monitor System] Module da duoc nap thanh cong.\n");
    return 0;
}

static void __exit monitor_exit(void) {
    flush_workqueue(monitor_wq);
    destroy_workqueue(monitor_wq);
    proc_remove(our_proc_file);
    printk(KERN_INFO "[Monitor System] Module da duoc go bo.\n");
}

module_init(monitor_init);
module_exit(monitor_exit);


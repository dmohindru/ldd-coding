#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/fs.h>
#include "circular_buffer.h"

#define DEFAULT_BUFFER_SIZE 50

MODULE_AUTHOR("Dhruv Mohindru");
MODULE_LICENSE("Dual BSD/GPL");

static int blocky_dev_major = 0;
static int blocky_dev_minor = 0;
static int blocky_buffer_size = DEFAULT_BUFFER_SIZE;

struct blocky_driver {
    struct circular_buffer *buffer;
    struct mutex mutex;
    struct cdev cdev;
    wait_queue_head_t writer_q, reader_q;
};

module_param(blocky_buffer_size, long, 0);

ssize_t blocky_dev_read(struct file *file_p, char __user *buf, size_t count, loff_t *f_pos);
ssize_t blocky_dev_write(struct file *file_p, const char __user *buf, size_t count, loff_t *f_pos);
int blocky_dev_open(struct inode *inode, struct file *file_p);
int blocky_dev_release(struct inode *inode, struct file *file_p);

struct file_operations blocky_fops = {
        .owner = THIS_MODULE,
        .read = blocky_dev_read,
        .write = blocky_dev_write,
        .open = blocky_dev_open,
        .release = blocky_dev_release
};

int blocky_dev_init_module(void) {}

void blocky_dev_cleanup_module(void) {}

module_init(blocky_dev_init_module);
module_exit(blocky_dev_cleanup_module);
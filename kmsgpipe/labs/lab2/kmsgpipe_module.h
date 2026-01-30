#ifndef _KMSGPIPE_MODULE_H_
#define _KMSGPIPE_MODULE_H_

#include <linux/fs.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include "kmsgpipe.h"

#define DEFAULT_DATA_SIZE 1024
#define DEFAULT_CAPCITY 10

typedef struct
{
    wait_queue_head_t writer_q, reader_q;
    atomic_t reader_waiting, writer_waiting;
    kmsgpipe_buffer_t ring_buffer;
    struct mutex mutex;
    struct cdev cdev;
} kmsgpipe_t;

int kmsgpipe_module_init(void);
void kmsgpipe_module_exit(void);
ssize_t kmsgpipe_read(struct file *file_p, char __user *buf, size_t count, loff_t *f_pos);
ssize_t kmsgpipe_write(struct file *file_p, const char __user *buf, size_t count, loff_t *f_pos);
int kmsgpipe_open(struct inode *inode, struct file *file_p);
int kmsgpipe_release(struct inode *inode, struct file *file_p);

#endif

#ifndef _KMSGPIPE_MODULE_H_
#define _KMSGPIPE_MODULE_H_

#include <linux/fs.h>
#include <linux/types.h>
#include <linux/uaccess.h>

int kmsgpipe_module_init(void);
void kmsgpipe_module_exit(void);
ssize_t kmsgpipe_read(struct file *file_p, char __user *buf, size_t count, loff_t *f_pos);
ssize_t kmsgpipe_write(struct file *file_p, const char __user *buf, size_t count, loff_t *f_pos);
int kmsgpipe_open(struct inode *inode, struct file *file_p);
int kmsgpipe_release(struct inode *inode, struct file *file_p);

#endif

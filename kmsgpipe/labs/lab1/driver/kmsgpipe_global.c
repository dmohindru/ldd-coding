#include <linux/module.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/slab.h>  /* kmalloc() */
#include <linux/fs.h>    /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/mutex.h>

#include <linux/uaccess.h> /* copy_*_user */

MODULE_AUTHOR("Dhruv Mohindru");
MODULE_LICENSE("Dual BSD/GPL");

static int kmsgpipe_char_major = 0;
static int kmsgpipe_char_minor = 0;
static dev_t kmsgpipe_devno;

#define KMSGPIPE_DEFAULT_BUFFER_SIZE 4096

struct kmsgpipe_char_driver
{
    void *data;
    unsigned long size;
    size_t data_len;
    struct mutex mutex;
    struct cdev char_dev;
};
typedef struct kmsgpipe_char_driver kmsgpipe_char_driver_t;

ssize_t kmsgpipe_read(struct file *file_p, char __user *buf, size_t count, loff_t *f_pos);
ssize_t kmsgpipe_write(struct file *file_p, const char __user *buf, size_t count, loff_t *f_pos);
int kmsgpipe_open(struct inode *inode, struct file *file_p);
int kmsgpipe_release(struct inode *inode, struct file *file_p);

kmsgpipe_char_driver_t *char_driver_p;

struct file_operations kmsgpipe_fops = {
    .owner = THIS_MODULE,
    .read = kmsgpipe_read,
    .write = kmsgpipe_write,
    .open = kmsgpipe_open,
    .release = kmsgpipe_release,
};

static int __init kmsgpipe_module_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&kmsgpipe_devno, 0, 1, "kmsgpipe_global");
    if (ret)
    {
        pr_err("kmsgpipe: alloc_chrdev_region failed: %d\n", ret);
        return ret;
    }

    kmsgpipe_char_major = MAJOR(kmsgpipe_devno);
    kmsgpipe_char_minor = MINOR(kmsgpipe_devno);

    char_driver_p = kzalloc(sizeof(*char_driver_p), GFP_KERNEL);
    if (!char_driver_p)
    {
        ret = -ENOMEM;
        unregister_chrdev_region(kmsgpipe_devno, 1);
        return ret;
    }

    /* allocate per-device buffer */
    char_driver_p->data = kzalloc(KMSGPIPE_DEFAULT_BUFFER_SIZE, GFP_KERNEL);
    if (!char_driver_p->data)
    {
        ret = -ENOMEM;
        kfree(char_driver_p);
        return ret;
    }

    /* size is the buffer capacity; data_len tracks how much of it is used */
    char_driver_p->size = KMSGPIPE_DEFAULT_BUFFER_SIZE;
    char_driver_p->data_len = 0;
    mutex_init(&char_driver_p->mutex);

    cdev_init(&char_driver_p->char_dev, &kmsgpipe_fops);
    ret = cdev_add(&char_driver_p->char_dev, kmsgpipe_devno, 1);
    if (ret)
    {
        pr_err("kmsgpipe: cdev_add failed: %d\n", ret);
        kfree(char_driver_p->data);
        return ret;
    }

    pr_info("kmsgpipe: module loaded (major=%d, minor=%d)\n", kmsgpipe_char_major, kmsgpipe_char_minor);
    return 0;
}

static void __exit kmsgpipe_module_exit(void)
{
    if (char_driver_p)
    {
        cdev_del(&char_driver_p->char_dev);
        kfree(char_driver_p->data);
        kfree(char_driver_p);
        char_driver_p = NULL;
    }
    unregister_chrdev_region(kmsgpipe_devno, 1);
    pr_info("kmsgpipe: module unloaded\n");
}

int kmsgpipe_open(struct inode *inode_p, struct file *file_p)
{
    kmsgpipe_char_driver_t *kmsgpipe_dev;
    pr_info("kmsgpipe_open: open() system call invoked\n");

    if (!inode_p || !inode_p->i_cdev)
    {
        pr_alert("kmsgpipe_open: Uninitialised inode pointer received\n");
        return -ENODEV;
    }

    kmsgpipe_dev = container_of(inode_p->i_cdev,
                                struct kmsgpipe_char_driver,
                                char_dev);
    file_p->private_data = kmsgpipe_dev;

    return 0;
}

int kmsgpipe_release(struct inode *inode_p, struct file *file_p)
{
    pr_info("kmsgpipe_release: release() system call invoked\n");

    /* For per-device memory ownership we do not free device memory here.
     * The device buffer is allocated during module init and freed during
     * module exit. Clear the per-file private pointer to avoid dangling
     * references. */
    if (file_p && file_p->private_data)
        file_p->private_data = NULL;

    return 0;
}

ssize_t kmsgpipe_write(struct file *file_p, const char __user *buf, size_t count, loff_t *f_pos)
{
    kmsgpipe_char_driver_t *dev;

    if (!file_p)
        return -EINVAL;

    dev = file_p->private_data;
    if (!dev || !dev->data)
        return -ENODEV;

    pr_info("kmsgpipe_write: write() system call invoked for %lu bytes\n", count);

    /* bounds check against buffer capacity */
    if (count > dev->size)
        return -EFAULT;

    /* protect device buffer and offsets */
    mutex_lock(&dev->mutex);

    /* clear previous contents and copy new data */
    memset(dev->data, 0, dev->size);
    if (copy_from_user(dev->data, buf, count))
    {
        mutex_unlock(&dev->mutex);
        return -EFAULT;
    }

    /* store current used length */
    dev->data_len = count;

    /* advance file position (use *f_pos, not file->f_pos) */
    if (f_pos)
        *f_pos += count;

    pr_info("kmsgpipe_write: wrote %lu bytes to device buffer\n", count);

    mutex_unlock(&dev->mutex);
    return count;
}

ssize_t kmsgpipe_read(struct file *file_p, char __user *buf, size_t count, loff_t *f_pos)
{
    kmsgpipe_char_driver_t *dev;
    size_t avail;

    if (!file_p)
        return -EINVAL;

    dev = file_p->private_data;
    if (!dev || !dev->data)
        return -ENODEV;

    pr_info("kmsgpipe_read: read() system call invoked for %lu bytes\n", count);

    mutex_lock(&dev->mutex);

    /* if file position beyond stored data, nothing to read */
    if (!dev->data_len || (f_pos && *f_pos >= (loff_t)dev->data_len))
    {
        mutex_unlock(&dev->mutex);
        return 0;
    }

    avail = dev->data_len - (f_pos ? (size_t)*f_pos : 0);
    if (count > avail)
        count = avail;

    if (copy_to_user(buf, dev->data + (f_pos ? *f_pos : 0), count))
    {
        mutex_unlock(&dev->mutex);
        return -EFAULT;
    }

    /* advance file position */
    if (f_pos)
        *f_pos += count;

    mutex_unlock(&dev->mutex);

    return count;
}
module_init(kmsgpipe_module_init);
module_exit(kmsgpipe_module_exit);

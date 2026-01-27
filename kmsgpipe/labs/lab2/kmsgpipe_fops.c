#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/ktime.h>

#include <linux/kernel.h>
#include <linux/slab.h>  /* kmalloc() */
#include <linux/errno.h> /* error codes */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/mutex.h>

#include "kmsgpipe_module.h"
#include "kmsgpipe.h"

MODULE_AUTHOR("Dhruv Mohindru");
MODULE_LICENSE("Dual BSD/GPL");

static kmsgpipe_t *kmsgpipe_p;

static int kmsgpipe_char_major = 0;
static int kmsgpipe_char_minor = 0;
static dev_t kmsgpipe_devno;

/* Parameters */
static int data_size = DEFAULT_DATA_SIZE;
static int capacity = DEFAULT_CAPCITY;

module_param(data_size, int, 0);
module_param(capacity, int, 0);

ssize_t kmsgpipe_read(struct file *file_p, char __user *buf, size_t count, loff_t *f_pos);
ssize_t kmsgpipe_write(struct file *file_p, const char __user *buf, size_t count, loff_t *f_pos);
int kmsgpipe_open(struct inode *inode, struct file *file_p);
int kmsgpipe_release(struct inode *inode, struct file *file_p);

struct file_operations kmsgpipe_fops = {
    .owner = THIS_MODULE,
    .read = kmsgpipe_read,
    .write = kmsgpipe_write,
    .open = kmsgpipe_open,
    .release = kmsgpipe_release,
};

int kmsgpipe_module_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&kmsgpipe_devno, 0, 1, "kmsgpipe_lab2");
    if (ret)
    {
        pr_err("kmsgpipe: alloc_chrdev_region failed: %d\n", ret);
        return ret;
    }

    kmsgpipe_char_major = MAJOR(kmsgpipe_devno);
    kmsgpipe_char_minor = MINOR(kmsgpipe_devno);

    kmsgpipe_p = kzalloc(sizeof(*kmsgpipe_p), GFP_KERNEL);
    if (!kmsgpipe_p)
    {
        ret = -ENOMEM;
        unregister_chrdev_region(kmsgpipe_devno, 1);
        return ret;
    }

    /* allocate per-device buffer */
    /* allocate memory for base buffer*/

    /* allocate memory for kmsg records metadata*/
    uint8_t *base_buffer_p = kzalloc(data_size * capacity, GFP_KERNEL);
    if (!base_buffer_p)
    {
        ret = -ENOMEM;
        kfree(kmsgpipe_p);
        return ret;
    }

    kmsg_record_t *records_buffer_p = kzalloc(sizeof(kmsg_record_t) * capacity, GFP_KERNEL);
    if (!records_buffer_p)
    {
        ret = -ENOMEM;
        kfree(base_buffer_p);
        kfree(kmsgpipe_p);
        return ret;
    }

    kmsgpipe_p->ring_buffer.base = base_buffer_p;
    kmsgpipe_p->ring_buffer.records = records_buffer_p;

    /* Initialize ring buffer */
    kmsgpipe_init(kmsgpipe_p, base_buffer_p, records_buffer_p, capacity, data_size);

    /* Initialize wait queues */
    init_waitqueue_head(&kmsgpipe_p->reader_q);
    init_waitqueue_head(&kmsgpipe_p->writer_q);

    /* Initialize mutex*/
    mutex_init(&kmsgpipe_p->mutex);

    cdev_init(&kmsgpipe_p->cdev, &kmsgpipe_fops);
    kmsgpipe_p->cdev.owner = THIS_MODULE;
    ret = cdev_add(&kmsgpipe_p->cdev, kmsgpipe_devno, 1);
    if (ret)
    {
        pr_err("kmsgpipe: cdev_add failed: %d\n", ret);
        kfree(base_buffer_p);
        kfree(records_buffer_p);
        kfree(kmsgpipe_p);
        return ret;
    }

    pr_info("kmsgpipe: module loaded (major=%d, minor=%d)\n", kmsgpipe_char_major, kmsgpipe_char_minor);
    return 0;
}

void kmsgpipe_module_exit(void)
{
    if (kmsgpipe_p)
    {
        cdev_del(&kmsgpipe_p->cdev);
        kfree(kmsgpipe_p->ring_buffer.base);
        kfree(kmsgpipe_p->ring_buffer.records);
        kfree(kmsgpipe_p);
        kmsgpipe_p = NULL;
    }
    unregister_chrdev_region(kmsgpipe_devno, 1);
    pr_info("kmsgpipe: module unloaded\n");
}

int kmsgpipe_open(struct inode *inode_p, struct file *file_p)
{
    kmsgpipe_t *kmsgpipe_dev;
    pr_info("kmsgpipe_open: open() system call invoked\n");

    if (!inode_p || !inode_p->i_cdev)
    {
        pr_alert("kmsgpipe_open: Uninitialized inode pointer received\n");
        return -ENODEV;
    }

    kmsgpipe_dev = container_of(inode_p->i_cdev,
                                kmsgpipe_t,
                                cdev);
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
    kmsgpipe_t *dev;

    if (!file_p)
        return -EINVAL;

    dev = file_p->private_data;
    if (!dev)
        return -ENODEV;

    while (dev->ring_buffer.head == dev->ring_buffer.tail)
    {
        mutex_unlock(&dev->mutex);
        pr_info("\"%s\" writing: going to sleep\n", current->comm);
        if (wait_event_interruptible(dev->writer_q, (dev->ring_buffer.head != dev->ring_buffer.tail)))
        {
            return -ERESTARTSYS;
        }
        if (mutex_lock_interruptible(&dev->mutex))
        {
            return -ERESTARTSYS;
        }
    }
    /* Return error if writer tries to write with a data size greater than allowed data_size*/
    if (count > dev->ring_buffer.data_size)
    {
        return -EINVAL;
    }
    count = min(count, dev->ring_buffer.data_size);
    kuid_t uid = current_uid();
    kgid_t gid = current_gid();
    ktime_t timestamp = ktime_get();

    /* Allocate buffer to read user data into kernel buffer */
    const char *data = kzalloc(count, GFP_KERNEL);
    if (copy_from_user(data, buf, count))
    {
        mutex_unlock(&dev->mutex);
        return -EFAULT;
    }

    ssize_t op_res = kmsgpipe_push(&dev->ring_buffer, data, count, uid, gid, timestamp);

    if (op_res < 0)
    {
        pr_err("kmsgpipe_write: error pushing data from circular buffer");
    }
    else
    {
        /* We got some data pushed to circular buffer wake up any sleeping readers */
        wake_up_interruptible(&dev->reader_q);
    }

    return op_res;
}

ssize_t kmsgpipe_read(struct file *file_p, char __user *buf, size_t count, loff_t *f_pos)
{
    struct kmsgpipe_char_driver *dev;
    char *kbuf;
    ssize_t ret;

    if (!file_p)
        return -EINVAL;

    dev = file_p->private_data;
    if (!dev)
        return -ENODEV;

    kbuf = kmalloc(count, GFP_KERNEL);
    if (!kbuf)
        return -ENOMEM;

    ret = kmsgpipe_read_core(dev, kbuf, count, f_pos);
    if (ret > 0)
    {
        if (copy_to_user(buf, kbuf, ret))
            ret = -EFAULT;
    }

    kfree(kbuf);
    return ret;
}

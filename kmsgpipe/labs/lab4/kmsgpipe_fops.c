#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/ktime.h>

/* For debug fs */
#include <linux/debugfs.h>
#include <linux/seq_file.h>

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
#include "kmsgpipe_ioctl.h"

MODULE_AUTHOR("Dhruv Mohindru");
MODULE_LICENSE("Dual BSD/GPL");

static kmsgpipe_t *kmsgpipe_p;
static struct dentry *kmsgpipe_debugfs_root;

static int kmsgpipe_char_major = 0;
static int kmsgpipe_char_minor = 0;
static dev_t kmsgpipe_devno;
static long expiry_ms = DEFAULT_EXPIRY_MS;

/* Parameters */
static int data_size = DEFAULT_DATA_SIZE;
static int capacity = DEFAULT_CAPCITY;

module_param(data_size, int, 0);
module_param(capacity, int, 0);

ssize_t kmsgpipe_read(struct file *file_p, char __user *buf, size_t count, loff_t *f_pos);
ssize_t kmsgpipe_write(struct file *file_p, const char __user *buf, size_t count, loff_t *f_pos);
int kmsgpipe_open(struct inode *inode, struct file *file_p);
int kmsgpipe_release(struct inode *inode, struct file *file_p);
long kmsgpipe_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
/* Function for debug fs support */
int ksmgpipe_stats_show(struct seq_file *m, void *v);
int kmsgpipe_stats_open(struct inode *inode, struct file *file);
void kmsgpipe_cleanup_worker(struct work_struct *work);

struct file_operations kmsgpipe_fops = {
    .owner = THIS_MODULE,
    .read = kmsgpipe_read,
    .write = kmsgpipe_write,
    .open = kmsgpipe_open,
    .unlocked_ioctl = kmsgpipe_ioctl,
    .release = kmsgpipe_release,
};

struct file_operations kmsgpipe_stats_fops = {
    .owner = THIS_MODULE,
    .open = kmsgpipe_stats_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release};

int kmsgpipe_module_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&kmsgpipe_devno, 0, 1, "kmsgpipe_lab4");
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
    kmsgpipe_init(&kmsgpipe_p->ring_buffer, base_buffer_p, records_buffer_p, capacity, data_size);

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

    /* Initialize debug fs entry*/
    kmsgpipe_debugfs_root = debugfs_create_dir("kmsgpipe", NULL);
    if (!kmsgpipe_debugfs_root)
    {
        pr_warn("kmsgpipe: debugfs not available\n");
    }
    else
    {
        debugfs_create_file("stats", 0444, kmsgpipe_debugfs_root, NULL, &kmsgpipe_stats_fops);
    }

    /* Initialise delay worker, and schedule it */
    INIT_DELAYED_WORK(&kmsgpipe_p->kmsg_delayed_work, kmsgpipe_cleanup_worker);
    schedule_delayed_work(&kmsgpipe_p->kmsg_delayed_work, msecs_to_jiffies(expiry_ms));

    pr_info("kmsgpipe: module loaded (major=%d, minor=%d) and (data_size=%d, capacity=%d)\n", kmsgpipe_char_major, kmsgpipe_char_minor, data_size, capacity);
    return 0;
}

void kmsgpipe_module_exit(void)
{
    debugfs_remove_recursive(kmsgpipe_debugfs_root);
    if (kmsgpipe_p)
    {
        cancel_delayed_work_sync(&kmsgpipe_p->kmsg_delayed_work);
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

    kmsgpipe_t *dev_p;
    ssize_t op_res;
    int ret;

    if (!file_p)
        return -EINVAL;

    dev_p = file_p->private_data;
    if (!dev_p)
        return -ENODEV;
    /* Return error if writer tries to write with a data size greater than allowed data_size*/
    if (count > dev_p->ring_buffer.data_size)
    {
        return -EINVAL;
    }
    count = min(count, dev_p->ring_buffer.data_size);

    /* Scratch buffer */
    uint8_t *data = kzalloc(count, GFP_KERNEL);
    if (!data)
    {
        return -ENOMEM;
    }

    if (mutex_lock_interruptible(&dev_p->mutex))
    {
        kfree(data);
        return -ERESTARTSYS;
    }

    while (kmsgpipe_get_message_count(&dev_p->ring_buffer) == dev_p->ring_buffer.capacity)
    {
        mutex_unlock(&dev_p->mutex);
        if (file_p->f_flags & O_NONBLOCK)
        {
            return -EAGAIN;
        }
        atomic_inc(&dev_p->writer_waiting);
        ret = wait_event_interruptible(
            dev_p->writer_q,
            (kmsgpipe_get_message_count(&dev_p->ring_buffer) < dev_p->ring_buffer.capacity));
        atomic_dec(&dev_p->writer_waiting);
        if (ret)
        {
            return -ERESTARTSYS;
        }
        if (mutex_lock_interruptible(&dev_p->mutex))
        {
            return -ERESTARTSYS;
        }
    }

    uid_t uid = from_kuid(&init_user_ns, current_uid());
    gid_t gid = from_kgid(&init_user_ns, current_gid());
    ktime_t timestamp = ktime_get();

    if (copy_from_user(data, buf, count))
    {
        mutex_unlock(&dev_p->mutex);
        kfree(data);
        return -EFAULT;
    }

    op_res = kmsgpipe_push(&dev_p->ring_buffer, data, count, uid, gid, timestamp);

    if (op_res < 0)
    {
        pr_err("kmsgpipe_write: error pushing data from circular buffer");
    }
    else
    {
        /* We got some data pushed to circular buffer wake up any sleeping readers */
        wake_up_interruptible(&dev_p->reader_q);
    }

    mutex_unlock(&dev_p->mutex);
    /* free temporary allocated buffer */
    kfree(data);
    return op_res;
}

ssize_t kmsgpipe_read(struct file *file_p, char __user *buf, size_t count, loff_t *f_pos)
{
    kmsgpipe_t *dev_p;
    ssize_t op_res;
    int ret;

    if (!file_p)
        return -EINVAL;

    dev_p = file_p->private_data;
    if (!dev_p)
        return -ENODEV;

    /* Return error if reader tries to read a data size greater than allowed data_size */
    if (count > dev_p->ring_buffer.data_size)
    {
        return -EINVAL;
    }

    count = min(count, dev_p->ring_buffer.data_size);

    /* Scratch buffer */
    uint8_t *out_buf = kzalloc(count, GFP_KERNEL);
    if (!out_buf)
    {
        return -ENOMEM;
    }

    if (mutex_lock_interruptible(&dev_p->mutex))
    {
        kfree(out_buf);
        return -ERESTARTSYS;
    }

    while (kmsgpipe_get_message_count(&dev_p->ring_buffer) == 0)
    {
        mutex_unlock(&dev_p->mutex);
        if (file_p->f_flags & O_NONBLOCK)
        {
            return -EAGAIN;
        }
        atomic_inc(&dev_p->reader_waiting);
        ret = wait_event_interruptible(dev_p->reader_q, (kmsgpipe_get_message_count(&dev_p->ring_buffer) > 0));
        atomic_dec(&dev_p->reader_waiting);
        if (ret)
        {
            return -ERESTARTSYS;
        }
        if (mutex_lock_interruptible(&dev_p->mutex))
        {
            return -ERESTARTSYS;
        }
    }

    uid_t uid = from_kuid(&init_user_ns, current_uid());
    gid_t gid = from_kgid(&init_user_ns, current_gid());

    /* Allocate buffer to read data item from ring buffer */

    op_res = kmsgpipe_pop(&dev_p->ring_buffer, out_buf, uid, gid);

    if (op_res < 0)
    {
        pr_err("kmsgpipe_read: error poping data from circular buffer");
        kfree(out_buf);
        mutex_unlock(&dev_p->mutex);
        return op_res;
    }

    /* We popped some data from circular buffer wake up any sleeping writers */
    wake_up_interruptible(&dev_p->writer_q);

    if (copy_to_user(buf, out_buf, count))
    {
        mutex_unlock(&dev_p->mutex);
        kfree(out_buf);
        return -EFAULT;
    }

    mutex_unlock(&dev_p->mutex);
    /* free temporary allocated buffer */
    kfree(out_buf);
    return op_res;
}

int ksmgpipe_stats_show(struct seq_file *m, void *v)
{
    kmsgpipe_t *dev_p = kmsgpipe_p;
    ssize_t count;
    mutex_lock(&dev_p->mutex);
    count = kmsgpipe_get_message_count(&dev_p->ring_buffer);
    seq_printf(m, "capacity: %zu\n", dev_p->ring_buffer.capacity);
    seq_printf(m, "data_size: %zu\n", dev_p->ring_buffer.data_size);
    seq_printf(m, "message count: %zu\n", count);
    seq_printf(m, "readers waiting: %d\n", atomic_read(&dev_p->reader_waiting));
    seq_printf(m, "writers waiting: %d\n", atomic_read(&dev_p->writer_waiting));
    mutex_unlock(&dev_p->mutex);

    return 0;
}

int kmsgpipe_stats_open(struct inode *inode, struct file *file)
{
    return single_open(file, ksmgpipe_stats_show, inode->i_private);
}

long kmsgpipe_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    kmsgpipe_t *dev_p = kmsgpipe_p;
    long ret_val = 0, tmp;

    if (_IOC_TYPE(cmd) != KMSGPIPE_IOC_MAGIC)
        return -ENOTTY;
    if (_IOC_NR(cmd) > KMSGPIPE_IOC_MAXNR)
        return -ENOTTY;

    switch (cmd)
    {
    case KMSGPIPE_IOC_G_DATA_SIZE:
        ret_val = put_user(dev_p->ring_buffer.data_size, (long __user *)arg);
        break;

    case KMSGPIPE_IOC_G_CAPACITY:
        ret_val = put_user(dev_p->ring_buffer.capacity, (long __user *)arg);
        break;

    case KMSGPIPE_IOC_G_MSG_COUNT:
        ssize_t count = kmsgpipe_get_message_count(&dev_p->ring_buffer);
        ret_val = put_user(count, (long __user *)arg);
        break;

    case KMSGPIPE_IOC_G_READERS:
        ret_val = put_user(atomic_read(&dev_p->reader_waiting), (long __user *)arg);
        break;

    case KMSGPIPE_IOC_G_WRITERS:
        ret_val = put_user(atomic_read(&dev_p->writer_waiting), (long __user *)arg);
        break;
    case KMSGPIPE_IOC_G_EXPIRY_MS:
        ret_val = put_user(expiry_ms, (long __user *)arg);
        break;
    case KMSGPIPE_IOC_S_EXPIRY_MS:
        if (!capable(CAP_SYS_ADMIN))
            return -EPERM;
        ret_val = get_user(expiry_ms, (long __user *)arg);
        break;

    case KMSGPIPE_IOC_CLEAR:
        if (!capable(CAP_SYS_ADMIN))
            return -EPERM;
        tmp = kmsgpipe_clear(&dev_p->ring_buffer);
        ret_val = tmp < 0 ? tmp : 0;
        break;
    }

    return ret_val;
}

void kmsgpipe_cleanup_worker(struct work_struct *work)
{
    kmsgpipe_t *kmsgpipe_dev;
    ktime_t timestamp = ktime_get();

    kmsgpipe_dev = container_of(work,
                                kmsgpipe_t,
                                kmsg_delayed_work.work);

    if (mutex_lock_interruptible(&kmsgpipe_dev->mutex))
    {
        return;
    }

    kmsgpipe_cleanup_expired(&kmsgpipe_dev->ring_buffer, timestamp);
    wake_up_interruptible(&kmsgpipe_dev->writer_q);

    mutex_unlock(&kmsgpipe_dev->mutex);
    schedule_delayed_work(&kmsgpipe_dev->kmsg_delayed_work, msecs_to_jiffies(expiry_ms));
}
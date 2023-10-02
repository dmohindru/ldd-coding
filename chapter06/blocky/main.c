#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include "circular_buffer.h"

#define DEFAULT_BUFFER_SIZE 50

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Dhruv Mohindru");

static int blocky_dev_major = 0;
static int blocky_dev_minor = 0;
static int buffer_size = DEFAULT_BUFFER_SIZE;

struct blocky_driver {
    struct circular_buffer *buffer;
    struct mutex mutex;
    struct cdev cdev;
    wait_queue_head_t writer_q, reader_q;
};

module_param(buffer_size, int, 0);

ssize_t blocky_dev_read(struct file *file_p, char __user *buf, size_t count, loff_t *f_pos);
ssize_t blocky_dev_write(struct file *file_p, const char __user *buf, size_t count, loff_t *f_pos);
int blocky_dev_open(struct inode *inode, struct file *file_p);
int blocky_dev_release(struct inode *inode, struct file *file_p);

static struct blocky_driver *p_blocky;
static char *data_buf;
static struct circular_buffer *p_circular_buffer;

struct file_operations blocky_fops = {
        .owner = THIS_MODULE,
        .read = blocky_dev_read,
        .write = blocky_dev_write,
        .open = blocky_dev_open,
        .release = blocky_dev_release
};

int blocky_dev_open(struct inode *inode, struct file *file_p) {
    struct blocky_driver *p_driver;

    printk(KERN_INFO "open() system call invoked for blocky device\n");

    /*
     container_of(pointer, container_type, container_field);
     This macro takes a pointer to a field of type container_field , within a structure of type container_type
     and returns a pointer to the containing structure.
    */
    p_driver = container_of(inode->i_cdev, struct blocky_driver, cdev);
    file_p->private_data = p_driver;

    return 0;          /* success */
}

int blocky_dev_release(struct inode *inode, struct file *file_p) {
    printk(KERN_INFO "release() system call invoked for blocky driver\n");

    return 0;
}

ssize_t blocky_dev_read(struct file *file_p, char __user *buf, size_t count, loff_t *f_pos) {
    struct blocky_driver *dev = file_p->private_data;
    char data[count]; /* local buffer to copy data from circular buffer */
    if (mutex_lock_interruptible(&dev->mutex)) {
        return -ERESTARTSYS;
    }

    while (circular_buffer_length(dev->buffer) < count) { /* Not enough data available for the reader  */
        mutex_unlock(&dev->mutex);
        // TODO: Insert non blocking code here

        printk(KERN_INFO "Process %i (%s) going to sleep\n", current->pid, current->comm);
        if (wait_event_interruptible(dev->reader_q, circular_buffer_length(dev->buffer) >= count)) {
            return -ERESTARTSYS;
        }
        // Process has woken up reacquire lock to begin its read process
        printk("Process %i (%s) awoken\n", current->pid, current->comm);
        if (mutex_lock_interruptible(&dev->mutex)) {
            return -ERESTARTSYS;
        }
    }

    circular_buffer_read(dev->buffer, data, count);
    if (copy_to_user(buf, data, count)) {
        mutex_unlock(&dev->mutex);
        return -EFAULT;
    }

    mutex_unlock(&dev->mutex);

    // Wake up any sleeping writer waiting for space being available
    wake_up_interruptible(&dev->writer_q);

    printk(KERN_INFO "Process %i (%s) did read %li bytes\n", current->pid, current->comm, count);

    return count;
}

ssize_t blocky_dev_write(struct file *file_p, const char __user *buf, size_t count, loff_t *f_pos) {
    return 0;
}

int blocky_dev_init_module(void) {
    int result, dev_no, err;
    dev_t dev = 0;
    size_t data_buf_size;

    // allocate char driver region
    result = alloc_chrdev_region(&dev, blocky_dev_minor, 1, "blocky");

    // extract major number of driver allocated
    blocky_dev_major = MAJOR(dev);

    // check for error
    if (result < 0) {
        printk(KERN_INFO "failed to create calculator_dev device\n");
        return result;
    }

    // Allocate memory for blocky_driver struct
    p_blocky = (struct blocky_driver *)kmalloc(sizeof(struct blocky_driver), GFP_KERNEL);

    // zero out freshly allocated memory
    memset(p_blocky, 0, sizeof(struct blocky_driver));
    
    // Allocate memory for circular buffer struct
    p_circular_buffer = (struct circular_buffer *)kmalloc(sizeof(struct circular_buffer), GFP_KERNEL);
    
    // zero out freshly allocated memory
    memset(p_circular_buffer, 0, sizeof(struct circular_buffer));
    
    // Allocate memory for buffer managed by circular buffer
    data_buf_size = sizeof(char) * buffer_size;
    data_buf = kmalloc(data_buf_size, GFP_KERNEL);

    // Zero out data_buf
    memset(data_buf, 0, data_buf_size);
    
    // initialize circular buffer
    circular_buffer_init(p_circular_buffer, data_buf, data_buf_size);

    // assign circular buffer to blocky_driver struct
    p_blocky->buffer = p_circular_buffer;

    // init lock for concurrency safety
    mutex_init(&p_blocky->mutex);

    // initialise char driver
    dev_no = MKDEV(blocky_dev_major, blocky_dev_minor);
    cdev_init(&p_blocky->cdev, &blocky_fops);
    p_blocky->cdev.owner = THIS_MODULE;

    // add char driver to kernel
    err = cdev_add(&p_blocky->cdev, dev_no, 1);
    if (err) {
        printk(KERN_INFO "Error %d adding blocky driver (major: %d, minor: %d)\n",
               err,
               blocky_dev_major,
               blocky_dev_minor);
    }

    // announce drivers registration
    printk(KERN_INFO "Successfully registered blocky driver with kernel with circular buffer size of %lu bytes\n",
           data_buf_size);
    
    return result;
    
}

void blocky_dev_cleanup_module(void) {

    // remove char driver from the kernel
    cdev_del(&p_blocky->cdev);

    // remove allocated memory for the modules
    kfree(data_buf);
    kfree(p_circular_buffer);
    kfree(p_blocky);

    // unregister driver char dev region from kernel
    unregister_chrdev_region(MKDEV(blocky_dev_major, blocky_dev_minor), 1);

    // announce driver's de-registration
    printk(KERN_INFO "Successfully de-registered blocky driver with kernel\n");

}

module_init(blocky_dev_init_module);
module_exit(blocky_dev_cleanup_module);


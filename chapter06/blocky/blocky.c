#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/slab.h>
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

module_param(blocky_buffer_size, int, 0);

ssize_t blocky_dev_read(struct file *file_p, char __user *buf, size_t count, loff_t *f_pos);
ssize_t blocky_dev_write(struct file *file_p, const char __user *buf, size_t count, loff_t *f_pos);
int blocky_dev_open(struct inode *inode, struct file *file_p);
int blocky_dev_release(struct inode *inode, struct file *file_p);

struct blocky_driver *p_blocky;
char *data_buf;
struct circular_buffer *p_circular_buffer;

struct file_operations blocky_fops = {
        .owner = THIS_MODULE,
//        .read = blocky_dev_read,
//        .write = blocky_dev_write,
        .open = blocky_dev_open,
        .release = blocky_dev_release
};

int blocky_dev_open(struct inode *inode, struct file *file_p) {
    struct blocky_driver *p_driver;

    printk("open() system call invoked for blocky device\n");

    /*
     container_of(pointer, container_type, container_field);
     This macro takes a pointer to a field of type container_field , within a structure of type container_type
     and returns a pointer to the containing structure.
    */
    p_driver = container_of(inode->i_cdev, struct blocky_driver, cdev);
    file_p->private_data = p_driver;

    if ((file_p->f_flags & O_ACCMODE) == O_WRONLY) {
        if (mutex_lock_interruptible(&p_blocky->mutex))
            return -ERESTARTSYS;

        mutex_unlock(&p_blocky->mutex);
    }
    return 0;          /* success */
}

int blocky_dev_release(struct inode *inode, struct file *file_p) {
    printk("release() system call invoked for blocky driver\n");

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
        printk("failed to create calculator_dev device\n");
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
    data_buf_size = sizeof(char) * blocky_buffer_size;
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
        printk(KERN_NOTICE "Error %d adding blocky driver (major: %d, minor: %d)\n",
               err,
               blocky_dev_major,
               blocky_dev_minor);
    }

    // announce drivers registration
    printk(KERN_ALERT "Successfully registered blocky driver with kernel");
    
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
    printk(KERN_ALERT "Successfully de-registered calculator driver with kernel");

}

module_init(blocky_dev_init_module);
module_exit(blocky_dev_cleanup_module);
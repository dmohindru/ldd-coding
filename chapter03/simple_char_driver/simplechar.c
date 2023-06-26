#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>

#include <linux/uaccess.h>	/* copy_*_user */

#include <linux/uaccess.h>

// Module documentation
MODULE_AUTHOR("Dhruv Mohindru");
MODULE_LICENSE("Dual BSD/GPL");

// driver's major and minor number
static int simple_char_major = 0;
static int simple_char_minor = 0;

// Simple char driver sturcture
struct simple_char_driver {
    void *data;
    unsigned long size;
    struct mutex mutex;
    struct cdev cdev;
};

ssize_t simple_char_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
ssize_t simple_char_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
int simple_char_open(struct inode *inode, struct file *filp);
int simple_char_release(struct inode *inode, struct file *filp);

// keep the scope to current translation unit (this source file)
struct simple_char_driver *char_driver_p;



// file ops structure
struct file_operations simple_char_fops = {
        .owner =    THIS_MODULE,
        .read =     simple_char_read,
        .write =    simple_char_write,
        .open =     simple_char_open,
        .release =  simple_char_release
};

// file open operation function
int simple_char_open(struct inode *inode, struct file *filp) {

    struct simple_char_driver *dev;
    printk("open() system call invoked\n");
    /*
     container_of(pointer, container_type, container_field);
     This macro takes a pointer to a field of type container_field , within a structure of type container_type
     and returns a pointer to the containing structure.
    */
    dev = container_of(inode->i_cdev, struct simple_char_driver, cdev);
    filp->private_data = dev;

    if ((filp->f_flags & O_ACCMODE) == O_WRONLY) {
        if (mutex_lock_interruptible(&dev->mutex))
            return -ERESTARTSYS;

        // TODO: kfree is throwing an error
//        if (dev->data)
//            kfree(dev->data);
        mutex_unlock(&dev->mutex);
    }
    return 0;          /* success */
}

// file release operation function
int simple_char_release(struct inode *inode, struct file *filp) {
    printk("release() method invoked\n");

    return 0;
}

// file read operation function
ssize_t simple_char_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {


    size_t retval = 0;
    // Retrieve pointer to device structure previously set in open() system call
    struct simple_char_driver *dev = filp->private_data;
    if (mutex_lock_interruptible(&dev->mutex)) {
        return -ERESTARTSYS;
    }

    // print some statistics information
    printk("read() system call invoked for %ld bytes", count);
    printk("Content of buffer: %s", (char *)dev->data);
    printk("Size of device data: %ld", dev->size);
    printk("f_pos value %lld", *f_pos);


    if (*f_pos >= dev->size)
        goto out;

    // check if userland program requested for more data than available in kernel buffer
    if (dev->size < count) {
        retval = dev->size;
    }

    // copy data from kernel buffer to user buffer
    if (copy_to_user(buf, dev->data, retval)) {
        retval = -EFAULT;
        goto out;
    }

    // update file position
    *f_pos += retval;

    out:
    mutex_unlock(&dev->mutex);

    // IMPORTANT: read function should return 0 eventually otherwise library function will repeatedly call
    // file read function
    return retval;
}

// file write operation function
ssize_t simple_char_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {


    // Retrieve pointer to device structure previously set in open() system call
    struct simple_char_driver *dev = filp->private_data;

    size_t  retval = -ENOMEM;
    size_t byte_to_allocate;

    if (mutex_lock_interruptible(&dev->mutex))
        return -ERESTARTSYS;

    printk("write() system call invoked for %lu bytes\n", count);

    // if free previously allocated memory
    printk("Freeing previous data allocated\n");
    // TODO: kfree on valid data pointer is causing driver to fail, research about it
    //if (dev->data)
    //    kfree(dev->data);
    // reset size
    //dev->size = 0;

    printk("Allocating new memory");
    byte_to_allocate = sizeof (char) * count;
    // allocate requested bytes of data
    dev->data = kmalloc(byte_to_allocate, GFP_KERNEL);
    // zero out allocated memory
    memset(dev->data, 0, byte_to_allocate);
    printk("Allocated %lu bytes of memory", byte_to_allocate);

    // copy data from user space buffer to kernel buffer
    if (copy_from_user(dev->data, buf, count)) {
        retval = -EFAULT;
        goto out;
    }
    printk("copied data from user into kernel");

    // update size of device structure
    dev->size = count;

    // update file offset pointer
    *f_pos = count;


    printk("write() current value of f_pos= %lld\n", *f_pos);


    // set return value
    retval = count;

    out:
    mutex_unlock(&dev->mutex);
    return retval;
}

// driver's init function
int simple_char_init_module(void) {
    int result, devno, err;
    dev_t dev = 0;

    // allocate char driver region
    result = alloc_chrdev_region(&dev, simple_char_minor, 1, "simplechar");
    // extract major number of driver
    simple_char_major = MAJOR(dev);

    // Check for error
    if (result < 0) {
        printk( "simple_char driver cannot get major number %d\n", simple_char_major);
        return result;
    }

    // Allocate memory for simple_char_driver structure
    char_driver_p = (struct simple_char_driver*)kmalloc(sizeof (struct simple_char_driver), GFP_KERNEL);

    // zero out freshly allocated memory
    memset(char_driver_p, 0, sizeof(struct simple_char_driver));

    // acquire lock for concurrency safety
    mutex_init(&char_driver_p->mutex);

    // initialise char driver
    devno = MKDEV(simple_char_major, simple_char_minor);
    cdev_init(&char_driver_p->cdev, &simple_char_fops);
    char_driver_p->cdev.owner = THIS_MODULE;

    // add char driver to kernel
    err = cdev_add(&char_driver_p->cdev, devno, 1);
    if (err)
        printk(KERN_NOTICE "Error %d adding simple_char_driver (major: %d, minor: %d)\n",
               err,
               simple_char_major,
               simple_char_minor
               );

    // announce drivers registration
    printk("Successfully registered simplechar driver with kernel");

    return result;
}

// driver's exit function
void simple_char_cleanup_module(void) {
    // release if simple_char_driver hold any valid memory allocation
    if (char_driver_p->data)
        kfree(char_driver_p->data);

    // remove char driver from kernel
    cdev_del(&char_driver_p->cdev);

    // remove simple_char_driver allocated memory
    kfree(char_driver_p);

    // unregister driver char dev region from kernel
    unregister_chrdev_region(MKDEV(simple_char_major, simple_char_minor), 1);

    // announce driver's de-registration
    printk(KERN_ALERT "Successfully de-registered simplechar driver with kernel");
}

// register driver's init and exit function with kernel
module_init(simple_char_init_module);
module_exit(simple_char_cleanup_module);




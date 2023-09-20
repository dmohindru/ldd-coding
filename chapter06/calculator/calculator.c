#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include "calculator.h"

// Module documentation
MODULE_AUTHOR("Dhruv Mohindru");
MODULE_LICENSE("Dual BSD/GPL");

static int calculator_dev_major = 0;
static int calculator_dev_minor = 0;

static struct calculator_driver *calculator_driver_p;

ssize_t calculator_dev_read(struct file *file_p, char __user *buf, size_t count, loff_t *f_pos);
ssize_t calculator_dev_write(struct file *file_p, const char __user *buf, size_t count, loff_t *f_pos);
int calculator_dev_open(struct inode *inode, struct file *file_p);
int calculator_dev_release(struct inode *inode, struct file *file_p);
long calculator_dev_ioctl(struct file *file_p, unsigned int cmd, unsigned long arg);

struct file_operations calculator_fops = {
        .owner = THIS_MODULE,
        .read = calculator_dev_read,
        .write = calculator_dev_write,
        .open = calculator_dev_open,
        .release = calculator_dev_release,
        .unlocked_ioctl = calculator_dev_ioctl
};

int calculator_dev_open(struct inode *inode, struct file *file_p) {
    struct calculator_driver *cal_driver_p;
    printk("open() system call invoked for calculator device\n");

    /*
     container_of(pointer, container_type, container_field);
     This macro takes a pointer to a field of type container_field , within a structure of type container_type
     and returns a pointer to the containing structure.
    */
    cal_driver_p = container_of(inode->i_cdev, struct calculator_driver, cdev);
    file_p->private_data = cal_driver_p;

    if ((file_p->f_flags & O_ACCMODE) == O_WRONLY) {
        if (mutex_lock_interruptible(&cal_driver_p->mutex))
            return -ERESTARTSYS;

        mutex_unlock(&cal_driver_p->mutex);
    }
    return 0;          /* success */

}

int calculator_dev_release(struct inode *inode, struct file *file_p) {
    printk("release() system call invoked for calculator driver\n");

    return 0;
}

ssize_t calculator_dev_read(struct file *file_p, char __user *buf, size_t count, loff_t *f_pos) {

    // Retrieve pointer to device structure previously set in open() system call
    struct calculator_driver *dev = file_p->private_data;
    size_t retval = 0;
    size_t result_size = sizeof(dev->result);

    if (mutex_lock_interruptible(&dev->mutex)) {
        return -ERESTARTSYS;
    }

    // print some statistics information
    printk("read() system call invoked for %ld bytes", count);
    printk("Second operand: %ld", dev->second_op);
    printk("Operator: %d", dev->op);
    printk("f_pos value %lld", *f_pos);


    if (*f_pos >= result_size)
        goto out;

    // check if userland program requested for more data than available in kernel buffer
    if (result_size < count) {
        retval = result_size;
    }


    // copy data from kernel buffer to user buffer
    if (copy_to_user(buf, &dev->result, result_size)) {
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

ssize_t calculator_dev_write(struct file *file_p, const char __user *buf, size_t count, loff_t *f_pos) {
    
    // Retrieve pointer to device structure previously set in open() system call
    struct calculator_driver *dev = file_p->private_data;
    size_t required_size = sizeof(dev->first_op);
    printk("write() system call invoked for %ld bytes", count);
    
    if (count < required_size) {
        return -EINVAL;
    }

    if (mutex_lock_interruptible(&dev->mutex)) {
        return -ERESTARTSYS;
    }

    // copy only the required_size of bytes to first operand
    if (copy_from_user(&dev->first_op, buf, required_size)) {
        return -EFAULT;
    }

    printk("write(): value of first_op [%ld]\n", dev->first_op);

    // Perform business logic of the driver
    switch(dev->op) {
        case ADD:
            printk("write(): add operation being performed\n");
            dev->result = dev->first_op + dev->second_op;
            break;
        case SUB:
            printk("write(): sub operation being performed\n");
            dev->result = dev->first_op - dev->second_op;
            break;
        case MUL:
            printk("write(): mul operation being performed\n");
            dev->result = dev->first_op * dev->second_op;
            break;
        case DIV:
            printk("write(): div operation being performed\n");
            dev->result = dev->first_op / dev->second_op;
            break;
        default:
            printk("write(): No operation being performed\n");
    }

    printk("write(): value of result [%ld]\n", dev->result);

    // Important: don't update the value of f_pos pointer

    mutex_unlock(&dev->mutex);

    return required_size;

}

long calculator_dev_ioctl(struct file *file_p, unsigned int cmd, unsigned long arg) {



    // Retrieve pointer to device structure previously set in open() system call
    struct calculator_driver *dev = file_p->private_data;

    int err = 0, tmp;
    int retval = 0;



    // extract the type and number bitfield and don't decode
    // wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
    if (_IOC_TYPE(cmd) != CALCULATOR_IOC_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > CALCULATOR_IOC_MAX) return -ENOTTY;

    printk(KERN_INFO "Calculator ioctl function being invoked");
    // check if user process is allowed to access the provided memory address
    err = access_ok((void __user *) arg, _IOC_SIZE(cmd));
    if (!err) return -EFAULT;

    switch(cmd) {
        case CALCULATOR_SET_OPERATOR:
            printk(KERN_INFO "CALCULATOR_SET_OPERATOR ioctl command being invoked\n");
            if (!capable(CAP_SYS_ADMIN))
                return -EPERM;
            retval = __get_user(tmp, (int __user *)arg);
            if (tmp < ADD || tmp > DIV)
                return -EINVAL;
            dev->op = tmp;
            break;
        case CALCULATOR_SET_OPERAND:
            printk(KERN_INFO "CALCULATOR_SET_OPERAND ioctl command being invoked\n");
            retval = __get_user(dev->second_op, (long __user*)arg);
            break;
        case CALCULATOR_RESET:
            printk(KERN_INFO "CALCULATOR_RESET ioctl command being invoked\n");
            dev->second_op = 1;
            dev->op = ADD;
            break;
        case CALCULATOR_QUERY_OPERATOR:
            printk(KERN_INFO "CALCULATOR_QUERY_OPERATOR ioctl command being invoked\n");
            retval = __put_user(dev->op, (int __user *)arg);
            break;
        case CALCULATOR_QUERY_OPERAND:
            printk(KERN_INFO "CALCULATOR_QUERY_OPERAND ioctl command being invoked\n");
            retval = __put_user(dev->second_op, (long __user*)arg);
            break;
        default: // this block is redundant as cmd was checked against MAXNR
            return -ENOTTY;
    }

    return retval;

}


int calculator_dev_init_module(void) {
    int result, dev_no, err;
    dev_t dev = 0;

    // allocate char driver region
    result = alloc_chrdev_region(&dev, calculator_dev_minor, 1, "calculator");

    // extract major number of driver allocated
    calculator_dev_major = MAJOR(dev);

    // check for error
    if (result < 0) {
        printk("failed to create calculator_dev device\n");
        return result;
    }

    // Allocate memory for calculator_driver struct
    calculator_driver_p = (struct calculator_driver *)kmalloc(sizeof(struct calculator_driver), GFP_KERNEL);

    // zero out freshly allocated memory
    memset(calculator_driver_p, 0, sizeof(struct calculator_driver));

    // acquire lock for concurrency safety
    mutex_init(&calculator_driver_p->mutex);

    // initialise char driver
    dev_no = MKDEV(calculator_dev_major, calculator_dev_minor);
    cdev_init(&calculator_driver_p->cdev, &calculator_fops);
    calculator_driver_p->cdev.owner = THIS_MODULE;

    // initialise second operand and operator
    calculator_driver_p->second_op = 1;
    calculator_driver_p->op = ADD;

    // TODO just for debug purpose
    //calculator_driver_p->result = 1234;

    // add char driver to kernel
    err = cdev_add(&calculator_driver_p->cdev, dev_no, 1);
    if (err) {
        printk(KERN_NOTICE "Error %d adding calculator_driver (major: %d, minor: %d)\n",
               err,
               calculator_dev_major,
               calculator_dev_minor);
    }

    // announce drivers registration
    printk(KERN_ALERT "Successfully registered calculator_driver with kernel");

    // print ioctl number for user land programs
    printk(KERN_INFO "calculator ioctl numbers\n");
    printk(KERN_INFO "CALCULATOR_SET_OPERATOR: 0x%lX\n", CALCULATOR_SET_OPERATOR);
    printk(KERN_INFO "CALCULATOR_SET_OPERAND: 0x%lX\n", CALCULATOR_SET_OPERAND);
    printk(KERN_INFO "CALCULATOR_RESET: 0x%X\n", CALCULATOR_RESET);
    printk(KERN_INFO "CALCULATOR_QUERY_OPERATOR: 0x%lX\n", CALCULATOR_QUERY_OPERATOR);
    printk(KERN_INFO "CALCULATOR_QUERY_OPERAND: 0x%lX\n", CALCULATOR_QUERY_OPERAND);

    return result;

}

void calculator_dev_cleanup_module(void) {

    // TODO: clean up any allocated memory

    // remove char driver from the kernel
    cdev_del(&calculator_driver_p->cdev);

    // remove calculator device allocated memory
    kfree(calculator_driver_p);

    // unregister driver char dev region from kernel
    unregister_chrdev_region(MKDEV(calculator_dev_major, calculator_dev_minor), 1);

    // announce driver's de-registration
    printk(KERN_ALERT "Successfully de-registered calculator driver with kernel");


}

module_init(calculator_dev_init_module);
module_exit(calculator_dev_cleanup_module);

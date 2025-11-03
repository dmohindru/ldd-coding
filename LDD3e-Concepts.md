# Introduction

This file captures the notes and important concepts extracted from book Linux Device Driver 3e, essentially acting as a quick cheat sheet Targeted for Linux 6.6 LTS — Hybrid C + Rust Reference

---

# Chapter 1 — Introduction to Device Drivers

Device drivers are the bridge between user space and hardware.
They run in kernel space and expose an interface (usually via /dev/\* files) that user applications can call through system calls (open, read, write, ioctl, etc.).

## Key Concepts

- **Kernel vs User Space**: kernel has full access to hardware / memory; user space does not.
- **Driver Types:**
  - **Character Drivers** — byte stream devices (UART, I2C, SPI).
  - **Block Drivers** — storage devices (SD, SATA).
  - **Network Drivers** — packet-oriented.
- **Device Node:** created under /dev; represents interface to the driver.
- **Major / Minor Numbers:** kernel identifies driver and device instance.
- **Modular Kernel:** loadable modules (.ko) can be inserted or removed dynamically.

**Minimal Example (C)**

```c
// hello.c
#include <linux/module.h>
#include <linux/init.h>

static int __init hello_init(void) {
    pr_info("Hello, kernel world!\n");
    return 0;
}

static void __exit hello_exit(void) {
    pr_info("Goodbye, kernel world!\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dhruv");
MODULE_DESCRIPTION("Minimal example module for LDD3 reckoner");
```

Build and load

```bash
make -C /lib/modules/$(uname -r)/build M=$PWD modules
sudo insmod hello.ko
sudo rmmod hello
dmesg | tail
```

**Rust Version**

```rust
use kernel::prelude::*;

module! {
    type: HelloModule,
    name: b"hello_rust",
    author: b"Dhruv",
    description: b"Hello World in Rust Kernel",
    license: b"GPL",
}

struct HelloModule;

impl KernelModule for HelloModule {
    fn init() -> Result<Self> {
        pr_info!("Hello from Rust kernel world!\n");
        Ok(HelloModule)
    }
}
```

## Learning Notes

- Start every driver journey with a “Hello World” module to verify build → load → unload cycle.
- Understand that each module lives in kernel address space → bugs can crash the system.
- pr_info() replaces printk(KERN_INFO …) in newer kernels.
- All kernel code must be GPL-compatible if it exports symbols.

## Real-World Reference:

Look at `/drivers/char/random.c` for an example of a basic char driver implementation.

---

# Chapter 2 — Building and Loading Modules

Modules are built outside the main kernel tree but linked against its headers using kbuild.

**Key Files**

```makefile
# Makefile
obj-m := hello.o
```

Run

```bash
make -C /lib/modules/$(uname -r)/build M=$PWD modules
```

## Module Management Commands

| Command               | Action                     |
| --------------------- | -------------------------- |
| `insmod <module.ko>`  | Load module manually       |
| `modprobe <module>`   | Load module + dependencies |
| `rmmod <module>`      | Remove module              |
| `lsmod`               | List loaded modules        |
| `modinfo <module.ko>` | View module metadata       |

## Signing Modules (Linux 6.6)

When Secure Boot is enabled, modules must be signed:

```bash
scripts/sign-file sha256 key.priv key.x509 hello.ko
```

## Learning Notes

- modprobe reads /lib/modules/{ver}/modules.dep to resolve dependencies.
- MODULE_LICENSE("GPL") ensures kernel exports are accessible.
- Keep builds out-of-tree using the M= syntax to avoid polluting the kernel source.

## Real World:

All modern distro kernels use DKMS (Dynamic Kernel Module Support) to rebuild external modules automatically on kernel updates.

---

# Chapter 3 — Character Drivers

Character drivers transfer data as a stream of bytes and are registered with the kernel via `cdev`.

## Key Concepts

- **Device Registration:**

```c
static dev_t dev;
alloc_chrdev_region(&dev, 0, 1, "simple_char");
```

- **Character Device Structure:**

```c
static struct cdev c_dev;
cdev_init(&c_dev, &fops);
cdev_add(&c_dev, dev, 1);
```

- **File Operations:** Each open device file is associated with a set of callbacks (`file_operations`).

## Example C Driver

```c
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define BUF_SIZE 128
static char device_buffer[BUF_SIZE];
static struct cdev my_cdev;
static dev_t dev;

static ssize_t my_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    size_t bytes = min(len, (size_t)(BUF_SIZE - *off));
    if (copy_to_user(buf, device_buffer + *off, bytes))
        return -EFAULT;
    *off += bytes;
    return bytes;
}

static ssize_t my_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    size_t bytes = min(len, (size_t)(BUF_SIZE - *off));
    if (copy_from_user(device_buffer + *off, buf, bytes))
        return -EFAULT;
    *off += bytes;
    return bytes;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read  = my_read,
    .write = my_write,
};

static int __init my_init(void)
{
    alloc_chrdev_region(&dev, 0, 1, "simple_char");
    cdev_init(&my_cdev, &fops);
    cdev_add(&my_cdev, dev, 1);
    pr_info("Registered char dev %d:%d\n", MAJOR(dev), MINOR(dev));
    return 0;
}

static void __exit my_exit(void)
{
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("simple_char removed\n");
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
```

Create the device node:

```bash
sudo mknod /dev/simple_char c <major> 0
```

Test

```bash
echo "abc" | sudo tee /dev/simple_char
sudo cat /dev/simple_char
```

## Learning Notes

- copy_to_user / copy_from_user handle address space protection.
- Always check user-supplied lengths and pointers.
- Each char driver instance typically corresponds to one minor number.

## Rust Kernel Analogy

```rust
use kernel::file::FileOperations;

struct MyChar;
impl FileOperations for MyChar {
    kernel::declare_file_operations!(read, write);
    fn read(...) -> Result<usize> { /* similar logic */ }
}
```

---

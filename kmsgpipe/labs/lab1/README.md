# Lab 1

Instruction to

- Setup development environment
- Setup vscode ide for intellisense for kernel development
- Basic helloworld driver
- Basic kunit test

### Setup development environment

```bash
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r) \
git cmake pkg-config libgtest-dev python3
```

### Setup VScode for kernel development

Create a `c_cpp_properties.json` file `.vscode` folder at root of repository

```json
{
  "configurations": [
    {
      "name": "Linux Kernel Module",
      "includePath": [
        "${workspaceFolder}/**",
        "/usr/src/linux-headers-6.14.0-35-generic/include",
        "/usr/src/linux-headers-6.14.0-35-generic/include/uapi",
        "/usr/src/linux-headers-6.14.0-35-generic/include/generated",
        "/usr/src/linux-headers-6.14.0-35-generic/arch/x86/include",
        "/usr/src/linux-headers-6.14.0-35-generic/arch/x86/include/uapi",
        "/usr/src/linux-headers-6.14.0-35-generic/arch/x86/include/generated"
      ],
      "defines": ["__KERNEL__", "MODULE"],
      "compilerPath": "/usr/bin/gcc",
      "cStandard": "c11",
      "cppStandard": "c++17",
      "intelliSenseMode": "linux-gcc-x64"
    }
  ],
  "version": 4
}
```

# kmsgpipe_basic driver — Lab 1

## Driver 1 kmsgpipe_basic

- Buffer is stored at device level.
- Any process can write to the buffer. On write previous buffer is erased, and data is copied to the buffer. If request to write data is greater than buffer size throw error.
- All process will read from same buffer. Experiment how file read pointer behaves per process.

📚 Purpose

This driver is a simple character device used for the lab exercise. The README documents the important design decision used in this implementation: the driver currently allocates a single per-device buffer (shared by all opens of the device). A later lab will add a second driver that demonstrates per-open allocation (one buffer per open()).

💡 Key design points

- Per-device allocation: the driver allocates device memory once in `module_init()` and frees it in `module_exit()`.
  - Allocation: `char_driver_p = kzalloc(sizeof *char_driver_p, GFP_KERNEL);`
  - Buffer: `char_driver_p->data = kzalloc(KMSGPIPE_DEFAULT_BUFFER_SIZE, GFP_KERNEL);` (size stored in `char_driver_p->size`)
  - Mutex initialization: `mutex_init(&char_driver_p->mutex);`
  - `cdev` is initialized and added with `cdev_init()` / `cdev_add()`.
- Per-open state: this driver uses `file->private_data` to point to the device object (set in `open()` via `container_of()`), but it does not allocate per-open resources. The `release()` handler clears `file->private_data` but does not free the device buffer.

Why per-device allocation here?

- It is appropriate when the buffer is a shared resource maintained by the device (a single logical device), and concurrency is handled by locking (we use `mutex`).

When to use per-open allocation instead?

- If each open needs independent state or buffers (e.g., independent read cursors, private buffers), allocate per-open structures in `open()` and free them in `release()`.
- Be mindful: `dup()` and `fork()` share `struct file` (and therefore `file->private_data`) — per-open allocations are for cases where separate `open()` created instances (not when `dup()` is used).

🔧 Important code patterns in this driver

- Getting device structure from the `inode` in `open()`:

```c
kmsgpipe_dev = container_of(inode_p->i_cdev,
                            struct kmsgpipe_char_driver,
                            char_dev);
file_p->private_data = kmsgpipe_dev;
```

- Clearing per-file pointer in `release()` (no free of device memory):

```c
if (file_p && file_p->private_data)
    file_p->private_data = NULL;
```

- Per-device cleanup in module exit:

```c
cdev_del(&char_driver_p->char_dev);
kfree(char_driver_p->data);
kfree(char_driver_p);
```

🔐 Concurrency

- The device stores a `struct mutex mutex;`. Lock the mutex (e.g., `mutex_lock(&dev->mutex)`) around operations that access or modify shared device state (buffer, size, head/tail indices if you add them).

- **Per-open synchronization:** If you allocate per-open state in `open()` (stored in `file->private_data`), protect that per-open struct with its own mutex because multiple threads or duplicated descriptors (via `dup()`/`fork()`) can access the same `struct file` concurrently. A safe pattern is:

```c
struct kmsgpipe_open {
  struct mutex lock; /* protects this per-open state */
  char *buf;
  size_t len;
};

int kmsgpipe_open(...) {
  struct kmsgpipe_open *s = kzalloc(sizeof *s, GFP_KERNEL);
  if (!s)
    return -ENOMEM;
  mutex_init(&s->lock);
  file->private_data = s;
  return 0;
}

ssize_t kmsgpipe_write(..., struct file *file) {
  struct kmsgpipe_open *s = file->private_data;
  mutex_lock(&s->lock);
  /* modify s->buf / s->len safely */
  mutex_unlock(&s->lock);
  return written;
}

int kmsgpipe_release(...) {
  struct kmsgpipe_open *s = file->private_data;
  if (!s)
    return 0;
  /* wait for concurrent ops to finish */
  mutex_lock(&s->lock);
  mutex_unlock(&s->lock);
  kfree(s);
  file->private_data = NULL;
  return 0;
}
```

- Use `mutex` (sleeping lock) in contexts where sleeping is allowed; use spinlocks for atomic/interrupt contexts.

⚠️ Notes and gotchas

- Do not free per-device memory in `release()` when it is shared across opens — this will produce use-after-free bugs if multiple processes have the device open.
- If you switch to per-open allocation later, allocate the per-open struct in `open()` and store it in `file->private_data` and free it in `release()`.
- Pay attention to `dup()`/`fork()` semantics: descriptor duplication does not call `open()` again and shares `struct file`.

🚀 Build & test (quick)

From this directory (`labs/lab1/driver`):

```bash
# build the module
make

# insert the module
sudo make insmod

# check kernel logs for messages (to see major/minor and other prints)
dmesg | tail -n 20

# create device node if needed (replace MAJOR and MINOR with values from dmesg):
# sudo mknod /dev/kmsgpipe c MAJOR MINOR && sudo chmod 0666 /dev/kmsgpipe

# remove module
sudo make rmmod
```

🧭 What’s next (lab extension)

- Later in this lab series you will create another driver that demonstrates **per-open** memory allocation (allocate a buffer in `open()`, store it in `file->private_data`, and free it in `release()`). The README will be updated with that example when implemented.

If you'd like, I can add a short example patch that converts this driver to use per-open buffers (as a second driver file) so you can compare the two approaches directly.

## Per-process and per-open semantics (plain English)

When multiple processes interact with a single character device, the kernel provides two useful objects to distinguish calls:

- **`struct file *` (per-open file object):** Each successful `open()` normally creates a distinct kernel `struct file` instance. Your `open()` gets a `file_p` pointer for that open, and subsequent `read()`/`write()`/`release()` calls for that open receive the same `file_p`. Use `file->private_data` for per-open state that should be cleaned up in `release()`.

- **`struct inode *` (per-device descriptor):** `inode` identifies the device node itself. For a single device node (`/dev/kmsgpipe`), `inode->i_cdev` will point to the same `struct cdev` across opens. Use the `inode` or the global device object for shared device state (the per-device buffer used by this driver).

Special cases and gotchas:

- **`dup()` / `fork()` semantics:** `dup()` or `fork()` does not call `open()` again — they duplicate or copy file descriptors. Duplicated descriptors share the same `struct file` (so they also share `file->private_data`). `release()` is only called when the _last_ reference to that `struct file` is dropped (i.e., after the final `close()`/`fput()`).

- **Concurrency:** Driver functions (your `read()`/`write()`) can be invoked concurrently by different processes or threads. Protect shared device state (per-device buffer) with a device-level lock (e.g., `mutex_lock(&dev->mutex)`), and protect per-open state with a per-open lock if concurrent access to `file->private_data` is possible.

- **Distinguishing callers:** If you need to know which process made the call, inspect the current task (`current->pid`) or credentials (`current_uid()`), but avoid relying on process identity for correctness — prefer explicit per-open or per-device state and proper locking.

Short checklist:

- Use `file->private_data` for per-open allocations and free them in `release()`.
- Use a device struct (found via `container_of(inode->i_cdev, ...)`) for shared device memory and free it at module exit.
- Always synchronize access (mutexes for sleeping context, spinlocks for atomic) to avoid races and use-after-free.

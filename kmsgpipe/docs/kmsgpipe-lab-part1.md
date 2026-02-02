# 🧪 kmsgpipe Lab Plan — Part 1

**Project:** kmsgpipe — Kernel Message Pipe Facility  
**Kernel Target:** 6.6 LTS  
**Focus:** Hardware-independent driver concepts  
**Goal:** Build, test, and evolve `kmsgpipe` from a simple character driver into a message-driven kernel service.

---

## 📘 Overview

This lab plan breaks down **Part 1** of the kmsgpipe project into structured hands-on tasks.  
Each task builds on the previous one, reinforcing kernel programming concepts through real implementation work.

---

## 🧩 Learning Objectives

By the end of Part 1, you will:

- Understand kernel module loading/unloading.
- Implement a minimal character driver with `read()` and `write()`.
- Learn kernel logging and debugging tools.
- Explore kernel synchronization primitives (`mutex`, `wait queues`).
- Implement blocking/non-blocking I/O in a device context.
- Use `ioctl()` for runtime configuration.
- Manage deferred work using `workqueues` and timers.

---

## 🧱 Lab Structure

| Lab       | Title                      | Concept Focus                             | Estimated Time |
| --------- | -------------------------- | ----------------------------------------- | -------------- |
| **Lab 1** | Hello kmsgpipe             | Basic character driver skeleton           | 45 min         |
| **Lab 2** | Persistent Echo            | Kernel message buffering & dynamic memory | 60 min         |
| **Lab 3** | Wait Queues & Blocking I/O | Synchronization and concurrency           | 90 min         |
| **Lab 4** | ioctl Interface            | Control paths and privileged ops          | 90 min         |
| **Lab 5** | Workqueue Cleanup          | Deferred tasks & timers                   | 90 min         |

---

## 🧪 Lab 1: Hello kmsgpipe

### Objective

Implement a basic **character device** `/dev/kmsgpipe0` that echoes back written strings.

### Steps

1. Create a kernel module `kmsgpipe_basic.c`.
2. Register a character device using `alloc_chrdev_region()` and `cdev_add()`.
3. Implement minimal `open`, `read`, `write`, and `release` callbacks.
4. Use `printk()` to trace all operations.
5. Test using:
   ```bash
   sudo insmod kmsgpipe_basic.ko
   echo "test123" > /dev/kmsgpipe0
   cat /dev/kmsgpipe0
   sudo rmmod kmsgpipe_basic
   dmesg | tail
   ```

### Learning Notes

- Understand `file_operations` structure.
- Practice `copy_to_user()` and `copy_from_user()`.
- Learn how `dmesg` reflects kernel logs.

---

## 🧪 Lab 2: Persistent Echo

### Objective

Upgrade Lab 1 to maintain an in-memory **message buffer** using a linked list or circular queue.

### Steps

1. Define `struct kmsg` for each message (up to 256 bytes).
2. Allocate with `kmalloc()` and store in a linked list (`list_head`).
3. Support multiple write calls before a read.
4. Implement simple FIFO semantics.
5. Add module parameters: `max_msgs`, `msg_size`.

### Test

```bash
echo "msg1" > /dev/kmsgpipe0
echo "msg2" > /dev/kmsgpipe0
cat /dev/kmsgpipe0
```

### Learning Notes

- Learn kernel memory management.
- Manage lifetime of objects (`kmalloc`/`kfree`).
- Understand kernel linked list APIs.

---

## 🧪 Lab 3: Wait Queues & Blocking I/O

### Objective

Add **blocking semantics** for read/write.

### Steps

1. Introduce `wait_queue_head_t` for readers and writers.
2. On empty read queue, block using `wait_event_interruptible()`.
3. On full write queue, block similarly until a message is read.
4. Support non-blocking mode via `O_NONBLOCK` and return `-EAGAIN`.

### Learning Notes

- Deep dive into process sleep/wake cycles.
- Explore `schedule()` and `wake_up_interruptible()`.
- Learn safe concurrency with `mutex_lock()`.

---

## 🧪 Lab 4: ioctl Interface

### Objective

Introduce control paths via `unlocked_ioctl()`.

### Tasks

1. Define ioctl macros (`KMSGPIPE_GET_STATS`, `KMSGPIPE_CLEAR_QUEUE`, etc.).
2. Implement basic handlers with privilege checks using `capable(CAP_SYS_ADMIN)`.
3. Return runtime stats in a `struct kmsgpipe_stats`.
4. Add logging for each ioctl command.

### Learning Notes

- Understand control vs data plane.
- Implement privileged operations safely.
- Study `_IO`, `_IOR`, `_IOW`, and `_IOWR` macros.

---

## 🧪 Lab 5: Workqueue Cleanup

### Objective

Implement a background cleanup task that periodically clears expired messages.

### Steps

1. Add `struct work_struct cleanup_work` and `struct timer_list flush_timer`.
2. Use `INIT_WORK()` and `schedule_work()` for deferred cleanup.
3. Implement configurable cleanup interval via ioctl.
4. Log cleanup operations.

### Learning Notes

- Explore kernel threads, workqueues, and timers.
- Learn about `mod_timer()`, `del_timer_sync()`, and race conditions.
- Understand deferring heavy work from the fast path.

## 🧪 Lab 6: Introduce sysfs

For device control introduced in lab4 via ioctl interface, do the same via sysfs as well.

---

## 🧠 Capstone Exercise

Combine all labs into `kmsgpipe.c` implementing:

- Character device registration
- Dynamic message queue
- Wait queues
- ioctl handling
- Workqueue-based cleanup

Test using user-mode script:

```bash
for i in {1..10}; do echo "msg$i" > /dev/kmsgpipe0; done
cat /dev/kmsgpipe0
sudo ioctl /dev/kmsgpipe0 KMSGPIPE_GET_STATS
```

---

## 🧩 Next Steps

Part 2 will introduce:

- Asynchronous notifications (`fasync`)
- Procfs and Sysfs integration
- Kselftest automation
- DebugFS instrumentation

---

## ✅ Deliverables

- `kmsgpipe_basic.c` → Lab 1
- `kmsgpipe_buffered.c` → Lab 2
- `kmsgpipe_wait.c` → Lab 3
- `kmsgpipe_ioctl.c` → Lab 4
- `kmsgpipe_workqueue.c` → Lab 5
- `kmsgpipe_final.c` → Capstone integrated driver

---

## 🧭 Tips

- Use `checkpatch.pl` to enforce coding style.
- Use `dmesg` and `trace_printk()` for runtime debugging.
- Keep small modular commits per lab.
- Write small userland tests in C using `open`, `read`, `write`, `ioctl`.
- Document each stage in your repo under `docs/labs/`.

---

> 💡 **Final Thought:**  
> The goal is not just to “get it working,” but to _understand why it works_. Each lab is designed to teach a specific kernel subsystem concept — together, they form a complete understanding of Linux character driver internals.

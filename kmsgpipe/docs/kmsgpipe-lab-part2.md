# KMSGPIPE LAB PLAN — PART 2

**Advanced Concepts: Async I/O, Procfs/Sysfs, DebugFS**

## Overview

This part extends the `kmsgpipe` driver beyond simple read–write operations into a **fully featured, production-grade** kernel driver.  
You’ll integrate advanced kernel mechanisms like asynchronous I/O, non-blocking operations, workqueues, and system integration points (procfs/sysfs/debugfs).

These labs assume you have completed **Part 1** and have a working `kmsgpipe` character device supporting:

- basic read/write
- ioctls
- mutual exclusion and synchronization

---

## Learning Objectives

By the end of Part 2, you will:

1. Implement **asynchronous notification mechanisms** (poll/select/epoll, fasync, waitqueues).
2. Add **non-blocking I/O** and robust error handling paths.
3. Expose driver status and configuration via **procfs** and **sysfs**.
4. Introduce **workqueues** and **delayed work** for background housekeeping tasks.
5. Use **debugfs** for driver inspection and runtime debugging.

---

## Lab 1 — Async I/O and Non-blocking Support

### Goal

Enhance `kmsgpipe` to allow user applications to:

- perform non-blocking reads/writes
- use `select()` or `poll()` to wait for data availability
- receive asynchronous signals when data is ready

### Steps

1. Add support for the `poll()` file operation.
2. Implement `fasync` handling using `fasync_helper()` and `kill_fasync()`.
3. Use `wait_event_interruptible()` and `wake_up_interruptible()` in your read/write paths.
4. Test using:
   ```bash
   cat /dev/kmsgpipe &
   echo "test async" > /dev/kmsgpipe
   ```
5. Extend tests with a custom user program that uses select() or poll().

### Key APIs

- `poll_wait()`
- `wait_queue_head_t`
- `fasync_struct`
- `fasync_helper()`, `kill_fasync()`

---

## Lab 2 — Driver Information via Procfs and Sysfs

### Goal

Expose runtime driver state and configuration parameters.

## Steps

1. Create a /proc/kmsgpipe entry that shows:
   - current buffer usage
   - number of open file descriptors
   - last cleanup timestamp
2. Create a sysfs attribute under /sys/class/kmsgpipe/ to:
   - set maximum buffer size
   - toggle periodic cleanup
3. Implement show() and store() callbacks for sysfs attributes.

## Key APIs

- `proc_create()`, `seq_file` interface (`seq_printf`, `single_open`)
- `class_create()`, `device_create()`
- `device_attribute`, `DEVICE_ATTR()`

## Test Commands

```bash
cat /proc/kmsgpipe
echo 4096 > /sys/class/kmsgpipe/max_buf_size
```

---

## Lab 3 — Workqueues and Deferred Tasks

### Goal

Move periodic maintenance (cleanup, statistics refresh) into background work.

## Steps

1. Define a global `struct workqueue_struct *kmsgpipe_wq`.
2. Use `INIT_DELAYED_WORK()` to schedule cleanup tasks periodically.
3. Expose cleanup interval as a sysfs tunable.
4. Ensure all workqueue operations respect module unload (cancel, flush).

## Key APIs

- `create_workqueue()`, `destroy_workqueue()`
- `INIT_WORK()`, `INIT_DELAYED_WORK()`
- `queue_delayed_work()`, `flush_workqueue()`

## Test Commands

```bash
cat /proc/kmsgpipe
echo 1 > /sys/class/kmsgpipe/trigger_cleanup
```

---

## Lab 4 — DebugFS and Runtime Introspection

### Goal

Enable advanced debugging and performance counters via debugfs.

## Steps

1. Create `/sys/kernel/debug/kmsgpipe/`.
2. Add files:
   - stats: total reads, writes, ioctls
   - log: last 10 operations
3. Use `debugfs_create_file()` and `seq_file` to expose structured info.

## Key APIs

- `debugfs_create_dir()`, `debugfs_create_file()`
- `single_open()`, `seq_printf()`

---

## Lab 5 — Integration Test Suite

### Goal

Develop user-space tests verifying all aspects of your driver.

## Steps

1. Implement tests using C or Python (ctypes).

2. Cover:
   - blocking/non-blocking I/O
   - poll/select/fasync
   - procfs/sysfs/debugfs consistency
   - concurrent access scenarios
3. Automate tests with Makefile and shell scripts.

## Optional Challenge — Epoll and Zero-Copy Extensions

- Replace `poll()` with `epoll()` for scalable event handling.
- Explore `splice()` or `mmap()` for zero-copy I/O paths.

## Deliverables

| Item          | Description                                               |
| ------------- | --------------------------------------------------------- |
| Source Code   | Enhanced `kmsgpipe` with async, sysfs, debugfs            |
| Test Programs | User-level test suite covering all cases                  |
| Documentation | Updated README and driver usage guide                     |
| Analysis      | Write-up on how each feature improves kernel-level design |

---

## Expected Outcome

By the end of Part 2, your `kmsgpipe` driver will:

- operate concurrently and asynchronously
- expose metrics and runtime controls through multiple kernel interfaces
- follow best practices for modern driver design

You’ll now have a codebase ready for Part 3, which integrates `DMA, interrupts, and device-tree-based hardware binding.`

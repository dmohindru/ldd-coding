# 🧩 kmsgpipe Functional Specification  
**Kernel Message Pipe Facility**  
**Version:** 0.9  
**Author:** Dhruv Mohindru (with ChatGPT co-design)  
**Kernel Target:** 6.6 LTS  
**Scope:** Hardware-independent kernel driver learning project

---

## 1️⃣ Overview

`kmsgpipe` provides user processes with a **message-based communication channel** implemented as one or more character devices under `/dev/kmsgpipeN`.

Each device node represents a **bounded message queue** in kernel space.  
Processes can read/write using standard file operations, while privileged users can control runtime behaviour via `ioctl()`.

`kmsgpipe` behaves like an enhanced UNIX pipe — message-oriented, configurable, and introspectable — designed to teach Linux driver internals such as memory management, synchronization, and concurrency control.

---

## 2️⃣ Device Model

| Device Node | Description |
|--------------|-------------|
| `/dev/kmsgpipe0` | Default global pipe instance (created on module load) |
| `/dev/kmsgpipe1..N` | Additional dynamically created instances (via ioctl) |

Each instance corresponds to a kernel structure:

```c
struct kmsgpipe {
    struct list_head msg_queue;
    struct mutex lock;
    wait_queue_head_t readq, writeq;
    unsigned int msg_count;
    unsigned int max_msgs;
    unsigned int msg_size;
    struct work_struct cleanup_work;
    struct timer_list flush_timer;
};
```

---

## 3️⃣ Message Model

| Property | Description |
|-----------|-------------|
| **Type** | Raw byte messages (opaque payload) |
| **Default size** | 256 bytes |
| **Queue depth** | 128 messages |
| **Ordering** | FIFO |
| **Blocking** | Readers block when empty, writers block when full (unless O_NONBLOCK) |
| **Lifetime** | Optionally time-bounded (expired messages cleaned by background worker) |
| **Ownership** | Each message tagged with UID/GID of writer |

```c
struct kmsg {
    uid_t owner_uid;
    gid_t owner_gid;
    size_t len;
    ktime_t timestamp;
    char data[];
};
```

---

## 4️⃣ Read / Write Semantics

### Write

- If queue full and blocking mode: sleep on `writeq`.
- If queue full and non-blocking: return `-EAGAIN`.
- If `len > msg_size`: return `-EMSGSIZE`.
- On success: copy message from user, enqueue it, wake any waiting readers.

### Read

- If queue empty and blocking: sleep on `readq`.
- If queue empty and non-blocking: return `-EAGAIN`.
- On success: dequeue oldest message and copy to user.
- Returns message length.

---

## 5️⃣ Concurrency Rules

- Multiple readers/writers per instance.
- Access serialized by `mutex` and wait queues.
- Reference count ensures cleanup when last fd closes.
- Locking order:
  ```
  pipe->lock → msg->lock (if any)
  ```
- Workqueue/timer callbacks acquire mutex before queue access.

---

## 6️⃣ IOCTL Interface

| Macro | Description | Privilege | Notes |
|--------|--------------|------------|-------|
| `KMSGPIPE_GET_STATS` | Get runtime statistics | Any | Returns `struct kmsgpipe_stats` |
| `KMSGPIPE_CLEAR_QUEUE` | Flush all messages | CAP_SYS_ADMIN | Drops all queued messages |
| `KMSGPIPE_SET_PARAMS` | Set `msg_size` and `max_msgs` | CAP_SYS_ADMIN | Only when queue empty |
| `KMSGPIPE_TOGGLE_BLOCK` | Switch blocking/non-blocking | Any | Toggles `O_NONBLOCK` |
| `KMSGPIPE_FLUSH_PERIODIC` | Enable/disable periodic cleanup | CAP_SYS_ADMIN | Starts/stops timer/workqueue |
| `KMSGPIPE_CREATE_NEW` | Create `/dev/kmsgpipeN` | CAP_SYS_ADMIN | Returns new device ID |
| `KMSGPIPE_DUMP` | Copy all messages to user buffer | CAP_SYS_ADMIN | Debug aid |

All macros built using `_IO`, `_IOR`, `_IOW`.

---

## 7️⃣ Periodic Cleanup

- Optional cleanup thread or delayed work cleans messages older than `expiry_ms` (default 10 s).
- Logs cleanup activity via `printk(KERN_INFO "kmsgpipe%d cleaned %d msgs")`.
- Demonstrates `kthread_run()`, `msleep_interruptible()`, and `ktime_get()`.

---

## 8️⃣ Statistics and Debug Info

- **Procfs:** `/proc/kmsgpipe_stats`
- **Sysfs:** `/sys/class/kmsgpipe/pipeN/`

Example proc output:
```
pipe0: msgs=12, drops=0, expired=3, writers=2, readers=1
pipe1: msgs=0, drops=4, expired=0, writers=0, readers=0
```

Uses `seq_file` for safe iteration.

---

## 9️⃣ Error Handling Summary

| Scenario | Error | Meaning |
|-----------|--------|---------|
| Write to full queue (non-blocking) | `-EAGAIN` | Queue full |
| Read empty queue (non-blocking) | `-EAGAIN` | No data |
| Oversized write | `-EMSGSIZE` | Message too long |
| Unauthorized ioctl | `-EPERM` | Privilege required |
| Invalid ioctl | `-ENOTTY` | Unknown command |
| Invalid params | `-EINVAL` | Bad user input |

---

## 🔟 Example User Workflow

```bash
sudo insmod kmsgpipe.ko
echo "system ready" > /dev/kmsgpipe0
cat /dev/kmsgpipe0
# → system ready

sudo ioctl /dev/kmsgpipe0 KMSGPIPE_GET_STATS
sudo ioctl /dev/kmsgpipe0 KMSGPIPE_CLEAR_QUEUE
sudo ioctl /dev/kmsgpipe0 KMSGPIPE_FLUSH_PERIODIC enable
```

---

## 🧠 Learning Map

| Concept | Demonstrated In |
|----------|-----------------|
| Char-device framework | Device registration |
| Memory management | `kmalloc`, `vmalloc` |
| Concurrency control | Mutex, wait queues |
| Blocking I/O | `wait_event_interruptible()` |
| Non-blocking mode | `O_NONBLOCK` |
| Deferred work | Workqueues, timers |
| ioctl mechanics | `unlocked_ioctl()` |
| Privilege enforcement | `capable()` |
| Debug interfaces | procfs/sysfs |
| Async notification | `fasync_helper()` |
| Testing | kselftest / user harness |

---

## ✅ Summary

`kmsgpipe` serves as a single, extensible driver theme through which all **hardware-independent** kernel driver concepts can be explored.  
It begins as a simple echo driver and evolves into a full-featured, concurrency-aware kernel subsystem with proper synchronization, control paths, and diagnostic interfaces.

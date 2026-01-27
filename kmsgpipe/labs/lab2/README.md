# Lab 2

This lab demonstrates how to use

- load a module with a parameter
- implement a debug FS
- Sleep and Wake process
  To demonstrate these features this lab will this [kmsgpipe ring buffer](../../lib/README.md) to implement its business logic

# Implementation

### Module parameters

Module will take the parameters on a commandline when loading a kernel module. And the parameter will decide how many entries should be there in kmsgpipe ring buffer.

### Ring Buffer consumers/producers

kmsgpipe will expose its functionality via device file. Following features will be implemented

- Process reading from the device file will act as a consumer of ring buffer and will get the data pointed at the head. If the buffer is empty the process is put to sleep till the buffer get a data from a writer process.
- Process writing from the device file will act as a producer of ring buffer and will write the data at the tail of buffer. If the buffer is full the process is put to sleep till the space in buffer is made available by a consumer process.

### Debug FS

This driver will implement a debug fs to show the current state of device with all the entries of the device ring buffer

# Recommended reading

### ldd3e

- hellop
- scull_pipe

### Linux apis

init_waitqueue_head();
wake_up_interruptible();
wait_event_interruptible();
wait_queue_head_t;

### Params to the modules

- data_size: size of individual data items
- capacity: number of items held in a circular buffer

### Memory calculation

- base buffer: data_size \* capacity
- kms_record_t array size = sizeof(kmsg_record) \* capacity

### Pattern for read file ops

```text
scull_p_read()
  |
  | mutex_lock
  |
  | while buffer empty
  |   unlock
  |   wait_event_interruptible()  <-- task sleeps here
  |   (context switch)
  |
  | <---- wake_up_interruptible()
  |
  | resumes here
  | mutex_lock
  | while condition checked again
  |
  | read data
  | wake up sleeping writers
```

### Pattern for write file ops

```text
scull_p_write()
  |
  | mutex_lock
  |
  | while buffer full
  |   unlock
  |   wait_event_interruptible()  <-- task sleeps here
  |   (context switch)
  |
  | <---- wake_up_interruptible()
  |
  | resumes here
  | mutex_lock
  | while condition checked again
  |
  | write data
  | wake up sleeping readers
```

### Getting UID and GID of the calling process

```c
#include <linux/cred.h>

kuid_t uid = current_uid();
kgid_t gid = current_gid();
```

To print numeric value

```c
pr_info("uid=%u gid=%u\n",
        __kuid_val(uid),
        __kgid_val(gid));
```

### Getting current ktime stamp

```c
#include <linux/ktime.h>

ktime_t timestamp = ktime_get();

```

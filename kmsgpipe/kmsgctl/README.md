## Introduction

`kmsgctl` a command line tool to run test/control kmsgpipe char based linux device driver

## IOCTL mapping table for kmsgpipe driver

| ioctl                      | CLI                         |
| -------------------------- | --------------------------- |
| `KMSGPIPE_IOC_G_DATA_SIZE` | `kmsgctl get data-size`     |
| `KMSGPIPE_IOC_G_CAPACITY`  | `kmsgctl get capacity`      |
| `KMSGPIPE_IOC_G_MSG_COUNT` | `kmsgctl get msg-count`     |
| `KMSGPIPE_IOC_G_READERS`   | `kmsgctl get readers`       |
| `KMSGPIPE_IOC_G_WRITERS`   | `kmsgctl get writers`       |
| `KMSGPIPE_IOC_G_EXPIRY_MS` | `kmsgctl get expiry-ms`     |
| (all above)                | `kmsgctl stats`             |
| `KMSGPIPE_IOC_S_EXPIRY_MS` | `kmsgctl set expiry-ms <N>` |
| `KMSGPIPE_IOC_CLEAR`       | `kmsgctl clear`             |

## IOCTL CLI interface

**Command Structure**

```sh
kmsgctl [--device /dev/kmsgpipe0] <command> [args]

```

**Read-only commands**

```sh
kmsgctl get data-size
kmsgctl get capacity
kmsgctl get msg-count
kmsgctl get readers
kmsgctl get writers
kmsgctl get expiry-ms
```

**Grouped Status**

```sh
kmsgctl stats
```

**Output**

```sh
data_size      : 4096
capacity       : 8192
msg_count      : 12
waiting_readers: 2
waiting_writers: 0
```

**Privileged Commands**
Set Expiry

```sh
sudo kmsgctl set expiry-ms 5000
```

**Clear Ring Buffer**

```sh
sudo kmsgctl clear
```

## Stress Test CLI interface

`kmsgctl` is extended to act as a stress-testing and validation tool for the `kmsgpipe` Linux character device driver.

It supports multiple workload patterns, metrics collection, and validation modes to test correctness and performance of:

- Non-blocking I/O
- poll/select/epoll integration
- Wait queue wakeups
- Driver robustness under load

### New stress Command

```sh
kmsgctl stress <pattern> [options]
```

### Stress Patterns

**Thrash (Balanced load)**

```sh
kmsgctl stress thrash --writers 50 --readers 50 --duration 30
```

- Equal producers and consumers
- Buffer oscillates between full and empty

**Write Flood**

```sh
kmsgctl stress write-flood --writers 100 --readers 5
```

- Writers dominate
- Buffer mostly full

**Read Starvation**

```sh
kmsgctl stress read-starvation --writers 5 --readers 100
```

- Readers dominate
- Buffer mostly empty

**Bursty Traffic**

```sh
kmsgctl stress bursty --writers 20 --burst-size 100 --burst-delay 200ms
```

- Burst writes followed by idle periods

**Chaos (Random Cancellation)**

```sh
kmsgctl stress chaos --writers 20 --readers 20 --drop-rate 0.1
```

- Random task cancellation and FD reopen

### Common Options

```sh
--writers <N>
--readers <N>
--duration <seconds>
--msg-size <bytes>
--rate <ops/sec>
--device <path>
--nonblock
--verbose
--sample-stats <ms>
--verify
--mode <blocking|nonblocking>
--engine <direct|epoll>
```

### Metrics Output

Example:

```
Total Writes      : 1,200,000
Total Reads       : 1,199,850
EAGAIN (write)    : 45,000
EAGAIN (read)     : 12,000
Throughput        : 85,000 ops/sec
Avg Latency       : 1.2 ms
Stalls Detected   : 0
```

### Runtime Stats Sampling

```
[t=1s] msg_count=100 readers=5 writers=10
[t=2s] msg_count=0   readers=20 writers=0
```

### Verification Mode

```sh
kmsgctl stress thrash --verify
```

Message format:

```
[writer_id | sequence_number]
```

Checks:

- Missing messages
- Duplicates
- Corruption

### Engine Modes

**Direct**

- Plain read/write loops

**Epoll**

- Async I/O using Tokio (`AsyncFd`)

### Mode Options

```sh
--mode blocking
--mode nonblocking
```

### Stall Detection

Detects inactivity:

```
No reads for X seconds
No writes for X seconds
```

### Trace Mode

```
[W1] write → EAGAIN
[R3] read → 128 bytes
[EPOLL] fd ready
```

## Development Phases

### Phase 1

- Blocking I/O
- Basic correctness

### Phase 2

- Non-blocking I/O
- Handle `EAGAIN`

### Phase 3

- Tokio + epoll

### Phase 4

- Chaos testing

### Introduction

`kmsgctl` a command line tool to run test/control kmsgpipe char based linux device driver

### IOCTL mapping table for kmsgpipe driver

| ioctl                      | CLI                         |
| -------------------------- | --------------------------- |
| `KMSGPIPE_IOC_G_DATA_SIZE` | `kmsgctl get data-size`     |
| `KMSGPIPE_IOC_G_CAPACITY`  | `kmsgctl get capacity`      |
| `KMSGPIPE_IOC_G_MSG_COUNT` | `kmsgctl get msg-count`     |
| `KMSGPIPE_IOC_G_READERS`   | `kmsgctl get readers`       |
| `KMSGPIPE_IOC_G_WRITERS`   | `kmsgctl get writers`       |
| (all above)                | `kmsgctl stats`             |
| `KMSGPIPE_IOC_S_EXPIRY_MS` | `kmsgctl set expiry-ms <N>` |
| `KMSGPIPE_IOC_CLEAR`       | `kmsgctl clear`             |

### CLI interface

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

### To be deleted later

```rust
use std::os::unix::fs::OpenOptionsExt;
use std::os::unix::io::{AsRawFd, RawFd};
use std::sync::Arc;

use nix::errno::Errno;
use nix::unistd::{read, write};
use tokio::io::unix::AsyncFd;

const DEVICE: &str = "/dev/kmsgpipe0";

#[tokio::main]
async fn main() -> std::io::Result<()> {
    let writers = 2;
    let readers = 2;

    println!("Starting thrash test...");

    let mut handles = Vec::new();

    // Spawn writers
    for id in 0..writers {
        let fd = open_nonblocking()?;
        let async_fd = Arc::new(AsyncFd::new(fd)?);

        handles.push(tokio::spawn(writer_task(id, async_fd)));
    }

    // Spawn readers
    for id in 0..readers {
        let fd = open_nonblocking()?;
        let async_fd = Arc::new(AsyncFd::new(fd)?);

        handles.push(tokio::spawn(reader_task(id, async_fd)));
    }

    // Run forever (or add timeout later)
    for h in handles {
        let _ = h.await;
    }

    Ok(())
}

// 🔹 Open device in NONBLOCK mode
fn open_nonblocking() -> std::io::Result<std::fs::File> {
    std::fs::OpenOptions::new()
        .read(true)
        .write(true)
        .custom_flags(libc::O_NONBLOCK)
        .open(DEVICE)
}

// 🔁 Writer task
async fn writer_task(id: usize, async_fd: Arc<AsyncFd<std::fs::File>>) {
    let fd = async_fd.get_ref().as_raw_fd();
    let msg = format!("writer-{}\n", id);

    loop {
        let mut guard = async_fd.writable().await.unwrap();

        match write(fd, msg.as_bytes()) {
            Ok(n) => {
                println!("[W{}] wrote {} bytes", id, n);
            }
            Err(Errno::EAGAIN) => {
                println!("[W{}] EAGAIN (buffer full)", id);
                guard.clear_ready(); // 🔥 critical
                continue;
            }
            Err(e) => {
                eprintln!("[W{}] write error: {:?}", id, e);
                break;
            }
        }
    }
}

// 🔁 Reader task
async fn reader_task(id: usize, async_fd: Arc<AsyncFd<std::fs::File>>) {
    let fd = async_fd.get_ref().as_raw_fd();
    let mut buf = [0u8; 128];

    loop {
        let mut guard = async_fd.readable().await.unwrap();

        match read(fd, &mut buf) {
            Ok(n) if n > 0 => {
                let s = String::from_utf8_lossy(&buf[..n]);
                println!("[R{}] read: {}", id, s.trim());
            }
            Ok(_) => {
                // nothing read
            }
            Err(Errno::EAGAIN) => {
                println!("[R{}] EAGAIN (buffer empty)", id);
                guard.clear_ready(); // 🔥 critical
                continue;
            }
            Err(e) => {
                eprintln!("[R{}] read error: {:?}", id, e);
                break;
            }
        }
    }
}
```

## The Key Insight

You want to visualize:

```
Waiting → Wakeup → Active → Waiting
```

## 🧩 Suggested Dashboard Layout

+------------------- Writers -------------------+
| ID | State | Writes | EAGAIN |
|----|--------------------|--------|------------|
| W0 | WaitingWritable | 102 | 45 |
| W1 | Writing | 120 | 30 |
+----------------------------------------------+

+------------------- Readers -------------------+
| ID | State | Reads | EAGAIN |
|----|--------------------|--------|------------|
| R0 | WaitingReadable | 90 | 12 |
| R1 | Reading | 110 | 20 |
+----------------------------------------------+

## ✅ Better Model (Important Change)

Writers:

```
loop:
    wait until writable (epoll)
    write immediately
```

Reader:

```
loop:
    wait until readable (epoll)
    read immediately ()
```

## 🔥 If you want “fast vs slow”

Simulate it like this:

- Writers: no delay (fast)
- Readers: add small delay after successful read

```rust
tokio::time::sleep(Duration::from_millis(200)).await;
```

This creates:

- buffer filling → writers block
- readers slowly draining → wakeups happen

👉 MUCH better signal

## Timing recommendation

| Component             | Value          |
| --------------------- | -------------- |
| Writers               | no delay       |
| Readers               | **200–500 ms** |
| UI refresh            | **100–200 ms** |
| Max delay (if needed) | **1 second**   |

## Console dashboard library

🥇 Best Choice: ratatui (formerly tui-rs)

# Lab 5

This lab demonstrates the implementation of `.poll` fops in a driver that support system calls like

- select
- poll
- epoll

# Non-Blocking I/O, poll/select/epoll, and Reactor Pattern (Linux Drivers)

## 1. Core Concepts

### read()/write()

- Perform actual I/O on a single file descriptor (FD)
- Blocking mode:
  - Sleeps if operation cannot proceed
- Non-blocking mode (`O_NONBLOCK`):
  - Returns `-EAGAIN` instead of sleeping

### poll()/select()/epoll

- Do NOT perform I/O
- Ask: “Would read/write block?”
- Allow waiting on **multiple FDs simultaneously**
- Put the process to sleep efficiently (no busy waiting)

---

## 2. Key Difference

| Operation         | Purpose                      |
| ----------------- | ---------------------------- |
| read/write        | Do the work                  |
| poll/select/epoll | Decide when work can be done |

---

## 3. Reactor Pattern Mapping

Reactor pattern consists of:

1. Register interest in events
2. Wait for events
3. Handle events

### Linux Mapping:

- Reactor: `select/poll/epoll`
- Event source: driver `.poll`
- Notification: wait queues

---

## 4. Why Non-Blocking I/O is Required

In an event loop:

```
epoll_wait(...);
read(fd);
```

Problem:

- Even if `epoll` signals readiness, `read()` can still block due to race conditions

Solution:

- Use `O_NONBLOCK`

Correct pattern:

```
while (1) {
    epoll_wait(...);

    for each fd:
        while (read(fd) > 0)
            process();
}
```

---

## 5. Driver Responsibilities

### .read

- Implements actual data transfer
- Must support:
  - Blocking (sleep using wait queues)
  - Non-blocking (`-EAGAIN`)

### .poll

- Reports readiness
- Registers wait queues using `poll_wait`

### .fasync (optional)

- Sends signals (`SIGIO`) for async notification

---

## 6. Critical Invariant

If `.poll` reports a file descriptor as readable:

- `read()` must NOT block when `O_NONBLOCK` is set

This ensures correct interaction with event loops.

---

## 7. Why poll/select/epoll are Needed

Without them:

- Blocking read → only one FD handled
- Non-blocking read → busy waiting (CPU waste)

With them:

- Efficient sleep
- Multi-FD coordination
- Scalable event-driven design

---

## 8. Summary

- `read/write` = perform I/O
- `O_NONBLOCK` = prevent blocking
- `poll/select/epoll` = coordinate multiple FDs efficiently
- `.poll` + `O_NONBLOCK` together enable correct async behavior

---

## 9. Final Mental Model

- poll/select/epoll → “Is it safe to act?”
- read/write → “Perform the action”
- O_NONBLOCK → “Never block while acting”

All three together form the foundation of scalable I/O systems in Linux.

## 10. Final take away (Important)

If your driver does not respect O_NONBLOCK and always blocks in read/write, then it undermines the usefulness of .poll for event-driven programming. While .poll may still wake processes, it no longer guarantees that subsequent I/O won’t block—breaking the assumptions required for correct reactor-style designs.

# Examples

Simple kernel examples

- `drivers/char/random.c`
- `fs/pipe.c`
- `drivers/tty/tty_io.c`

# TODO

- user land code to test non blocking feature rust tokio. Design user land application.
- Edge-triggered vs level-triggered epoll (VERY important)
- Build a test harness to prove wakeups

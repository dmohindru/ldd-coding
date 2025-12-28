# Kernel Driver Design & Testability Guide

## (A Practical Mental Model for Writing Testable Linux Drivers)

> This document captures a **design mindset** for Linux kernel driver development
> with a focus on **testability (KUnit)**, **correctness**, and **long-term maintainability**.
>
> Treat this as a _living design contract_ between yourself and your future self.

---

## 1. The Core Mental Model

> **File operations translate. Core logic decides.**

Linux kernel drivers should be structured as **two clearly separated layers**:

```
Userspace boundary
┌─────────────────────────────┐
│ file_operations (glue)      │  ← uaccess, allocation, policy
│  - open                     │
│  - read                     │
│  - write                    │
│  - ioctl                    │
└──────────────┬──────────────┘
               │
               ▼
┌─────────────────────────────┐
│ core / engine / logic       │  ← state, buffers, invariants
│  - buffer management        │
│  - bounds checking          │
│  - offsets                  │
│  - state machines           │
└─────────────────────────────┘
```

---

## 2. Responsibilities by Layer

### 2.1 File Operations Layer (Glue Code)

This layer is the **userspace trust boundary**.

It is responsible for:

- Allocating temporary kernel buffers (`kmalloc`, `memdup_user`)
- Copying data:
  - `copy_from_user`
  - `copy_to_user`
- Validating userspace pointers
- Handling partial copies and faults
- Logging syscall entry/exit (`pr_debug`, `pr_info`)
- Translating kernel errors to syscall return values

**This layer should NOT contain business logic.**

Example responsibilities:

```c
kbuf = memdup_user(user_buf, count);
ret  = write_core(dev, kbuf, count, pos);
copy_to_user(user_buf, kbuf, ret);
```

---

### 2.2 Core Logic Layer

This layer contains **all the real behavior** of the driver.

It should:

- Operate only on **kernel memory**
- Never touch userspace
- Never allocate per-call buffers
- Be deterministic and side-effect minimal
- Be safe to call from KUnit tests

Allowed operations:

- `memcpy`, `memset`
- State updates
- Bounds checking
- Offset calculations
- Mutex/lock usage
- Device buffer management

Disallowed operations:

- `copy_to_user`
- `copy_from_user`
- `memdup_user`
- Userspace pointers (`__user`)
- Logging (except maybe `WARN_ON`)

---

## 3. Memory Ownership Rules

### Golden Rule

> **Core code may allocate _device memory_, but never _transaction memory_.**

### Device Memory (Allowed in Core)

- Long-lived buffers
- Rings, queues
- Device state
- Allocated at init / probe time

Example:

```c
dev->buffer = kmalloc(SZ_4K, GFP_KERNEL);
```

### Transaction Memory (NOT allowed in Core)

- Per-read/write buffers
- Temporary syscall-related allocations

Example (wrong in core):

```c
char *tmp = kmalloc(count, GFP_KERNEL);   // ❌
copy_from_user(tmp, buf, count);          // ❌
```

---

## 4. Why This Design Matters

### 4.1 Enables KUnit

- Core logic becomes unit-testable
- No need to mock `struct file`
- No need to mock userspace access
- Tests run in UML or QEMU safely

### 4.2 Improves Correctness

- Clear ownership rules
- Fewer error paths
- Easier reasoning about state
- Concurrency bugs become visible

### 4.3 Scales with Complexity

This model works for:

- Char drivers
- Block drivers
- Network stacks
- Filesystems
- DRM / GPU drivers

---

## 5. KUnit Testing Philosophy

### What KUnit Tests

- Buffer logic
- Bounds checks
- State transitions
- Error paths
- Offset handling
- Invariants

### What KUnit Does NOT Test

- Real hardware behavior
- DMA correctness
- Interrupt delivery
- Timing guarantees
- Electrical properties

> **KUnit tests kernel logic, not kernel reality.**

---

## 6. Practical Design Checklist

Before writing a function, ask:

| Question                            | Belongs Where |
| ----------------------------------- | ------------- |
| Does it touch userspace?            | file-ops      |
| Does it allocate per-call memory?   | file-ops      |
| Does it manage device state?        | core          |
| Does it implement protocol rules?   | core          |
| Does it copy bytes between buffers? | core          |
| Does it log syscall behavior?       | file-ops      |

If the answer is “both”, the function is doing too much.

---

## 7. Naming Conventions

Recommended pattern:

- `*_core()` — pure logic
- `*_ops()` or file ops — glue
- `*_init()` / `*_exit()` — lifecycle

Example:

```c
kmsgpipe_write()
kmsgpipe_write_core()
```

---

## 8. One-Sentence Rule (Commit This to Memory)

> **“File operations translate data and context; core logic enforces behavior and invariants.”**

---

## 9. Final Notes

- This structure does **not** slow development — it accelerates it
- Refactoring early pays exponential dividends
- Your future self (and reviewers) will thank you

This document is intentionally concise, opinionated, and practical.
Update it only when experience proves it wrong.

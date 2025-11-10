# KMSGPIPE LAB PLAN — PART 4

**Testing, Instrumentation, and Rust Porting**

## Overview

This part transitions `kmsgpipe` from a working kernel driver into a **production-quality, testable, and observable system component**.  
It introduces structured testing methodologies (unit, integration, and stress testing), runtime tracing, and partial Rust-based rewrites to demonstrate safety and modern kernel development workflows.

---

## Learning Objectives

After completing Part 4, you’ll be able to:

1. Develop automated test suites for Linux drivers (using `kunit`, userland tools, and kernel test frameworks).
2. Instrument and trace driver execution using `tracepoints`, `ftrace`, and `eBPF`.
3. Collect runtime statistics via `tracefs` and performance counters.
4. Write hybrid C–Rust modules (using `rust-for-linux` abstractions).
5. Integrate CI pipelines for module build and test automation.

---

## Lab 1 — Kernel Unit Testing with KUnit

### Goal

Set up **KUnit tests** for `kmsgpipe`’s internal logic, e.g. ring-buffer management and IOCTL validation.

### Steps

1. Create `kmsgpipe_kunit.c` inside the driver source tree.
2. Register tests with `kunit_test_suite()`.
3. Add coverage for:
   - buffer allocation/deallocation
   - message enqueue/dequeue logic
   - ioctl parameter validation
4. Integrate KUnit build with:
   ```bash
   make ARCH=x86_64 CONFIG_KUNIT=y CONFIG_KMSGPIPE_KUNIT_TEST=y
   ./tools/testing/kunit/kunit.py run kmsgpipe
   ```
5. Export results as TAP (Test Anything Protocol).

### Key APIs

- `KUNIT_CASE()`, `KUNIT_EXPECT_EQ()`, `KUNIT_ASSERT_NOT_ERR_OR_NULL()`
- `kunit_test_suite()`

---

## Lab 2 — Instrumentation and Tracing (ftrace, tracepoints)

### Goal

Instrument the driver for performance and event analysis.

### Steps

1. Add **custom tracepoints** to log:
   - read/write entry and exit
   - DMA completion
   - IRQ handling
2. Use ftrace to measure function call latency.
3. Enable via debugfs:

```bash
echo 1 > /sys/kernel/debug/tracing/events/kmsgpipe/enable
```

4. Record traces:

```bash
echo function > /sys/kernel/debug/tracing/current_tracer
cat /sys/kernel/debug/tracing/trace_pipe
```

## Key APIs

- `TRACE_EVENT()`, `trace_event_reg()`
- `trace_printk()`
- `ftrace` and `tracefs` interfaces

---

## Lab 3 — eBPF Hooks and Performance Counters

### Goal

Attach eBPF programs to your driver’s tracepoints for runtime analytics.

### Steps

1. Define eBPF probes for kmsgpipe:read_enter and kmsgpipe:write_exit.
2. Collect metrics like message size, latency, and error rates.
3. Visualize results using bpftrace or perf.

### Example bpftrace Script

```bash
bpftrace -e 'tracepoint:kmsgpipe:write_exit { @[comm] = count(); }'
```

### Deliverable

A `tools/` directory containing scripts for runtime inspection.

---

## Lab 4 — Rust Port: Core Buffer Logic

### Goal

Port a critical subset of your driver (ring buffer) to Rust for memory-safety guarantees.

### Steps

1. Set up Rust-enabled kernel build (requires 6.6+ and `rustc >= 1.76`).
2. Create `src/lib.rs` implementing a safe abstraction of:
   - ring buffer allocation
   - enqueue/dequeue operations
   - cleanup routines
3. Expose FFI bindings using:

```bash
#[no_mangle]
pub extern "C" fn kmsgpipe_buf_push(ptr: *mut u8, len: usize) -> i32 { ... }
```

4. Link Rust static library with C driver using:

```bash
rustc --crate-type staticlib src/lib.rs -o libkmsgpipe.a
```

5. Replace legacy C functions with Rust-backed equivalents in driver build.

### Key Concepts

- Rust FFI (`extern "C"`)
- Kernel crate structure
- `unsafe` boundary minimization
- Error handling via `Result<T, E>` mapped to Linux `-E*` codes

---

## Lab 5 — Continuous Integration (CI) for Kernel Modules

### Goal

Automate build and test for your driver with GitHub Actions or GitLab CI.

## Steps

1. Define CI pipeline:
   - Build kernel module for `x86_64` and `arm64` (cross-compile).
   - Run static analysis (`clang-format`, `sparse`, `cppcheck`).
   - Execute KUnit tests in headless mode.
2. Upload artifacts (.ko, logs, TAP output).
3. Optional: deploy tests on QEMU via CI runner.

### Example Workflow Snippet

```yaml
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - run: make -C /lib/modules/$(uname -r)/build M=$PWD modules
  test:
    steps:
      - run: ./tools/testing/kunit/kunit.py run kmsgpipe
```

---

### Lab 6 — Stress Testing and Fault Injection

## Goal

Validate stability and resilience under edge cases.

## Steps

1. Use stress-ng and fault-injection kernel features:

```bash
echo 1 > /sys/kernel/debug/fail_alloc/ignore-gfp-wait
stress-ng --class io --timeout 60s
```

2. Introduce artificial errors in read/write path.
3. Capture logs and verify driver recovers gracefully.

## Optional Challenge — Rust User-space Test Harness

Use a Rust-based CLI (via `nix` and `libc` crates) to:

- open `/dev/kmsgpipe`
- send/receive buffers
- issue ioctl commands safely
  Integrate with `cargo test` for end-to-end validation.

---

## Deliverables

| Item          | Description                             |
| ------------- | --------------------------------------- |
| KUnit Tests   | Comprehensive kernel-side test coverage |
| eBPF Tools    | Live performance tracing scripts        |
| Rust Code     | Partial rewrite of core logic with FFI  |
| CI/CD         | Automated build/test pipeline           |
| Documentation | Test and trace guide in `docs/tests/`   |

---

## Expected Outcome

After Part 4, you’ll have:

- A thoroughly tested, instrumented, and observable Linux driver
- A modular codebase mixing C and Rust safely
- Automated build/test integration
- Production-level visibility and stability practices

This concludes the core kmsgpipe lab series, making you ready to:

- extend into networking or USB frameworks,
- or evolve the driver into a full firmware–host communication subsystem.

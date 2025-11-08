# KMSGPIPE LAB PLAN — PART 3

**Hardware Integration: DMA, Interrupts, Platform Bus, and Device Tree**

## Overview

In Part 3, `kmsgpipe` evolves from a purely software abstraction into a **hardware-backed character driver** that interacts with real or simulated hardware resources.  
You’ll learn how the Linux kernel manages data transfer using **DMA**, handles **interrupts**, and maps drivers onto devices through the **platform bus** and **device tree**.

These labs connect the conceptual kernel work of Parts 1 and 2 with actual hardware-oriented driver development.

---

## Learning Objectives

After completing Part 3, you’ll be able to:

1. Register and probe platform devices and drivers.
2. Handle hardware interrupts and bottom-half processing.
3. Implement memory-mapped I/O (MMIO).
4. Use DMA for efficient data transfers.
5. Integrate the driver with the Device Tree for board-specific configuration.
6. Test the driver on emulated (QEMU) or physical dev boards (STM32, Micro:bit V2).

---

## Lab 1 — Platform Driver & Probing Basics

### Goal

Transform `kmsgpipe` into a **platform driver** managed through the kernel device model.

### Steps

1. Define a `platform_driver` with `.probe` and `.remove` callbacks.
2. Create a `platform_device` entry from user space (or via Device Tree overlay).
3. Move initialization logic (allocating buffers, registering char dev) into the `.probe()` handler.
4. Add cleanup logic to `.remove()`.
5. Test load/unload via `modprobe` and `rmmod`.

### Key APIs

- `struct platform_driver`, `platform_driver_register()`
- `platform_get_resource()`
- `devm_kzalloc()`, `devm_ioremap_resource()`

### Example Test

```bash
echo kmsgpipe_test > /sys/bus/platform/drivers/kmsgpipe/bind
dmesg | tail
```

---

## Lab 2 — Interrupt Handling

## Goal

Introduce interrupt-driven notification to signal data availability or cleanup completion.

### Steps

1. Define an IRQ resource in platform data or Device Tree.
2. Register ISR via `request_irq()`.
3. Use a **bottom half** via `tasklet` or `workqueue` for deferred processing.
4. Generate synthetic interrupts using a timer or user-triggered ioctl.
5. Add counters for interrupt events visible in debugfs.

### Key APIs

- `request_irq()`, `free_irq()`
- `irqreturn_t`, `IRQ_HANDLED`
- `tasklet_init()`, `tasklet_schedule()`
- `spin_lock_irqsave()`, `spin_unlock_irqrestore()`

### Test

```bash
echo trigger_irq > /dev/kmsgpipe
cat /sys/kernel/debug/kmsgpipe/irq_stats
```

---

## Lab 3 — Memory-Mapped I/O (MMIO)

### Goal

Access simulated device registers and integrate them into kmsgpipe operations.

### Steps

1. Use `ioremap()` or `devm_ioremap_resource()` to map registers.
2. Implement read/write helpers using `iowrite32()` and `ioread32()`.
3. Simulate status and control registers for:
   - TX buffer status
   - RX buffer ready
   - IRQ status
4. Integrate register operations into existing I/O paths and ioctls.

### Key APIs

- `ioremap()`, `iounmap()`
- `iowrite32()`, `ioread32()`
- `wmb()`, `rmb()`, `barrier()`

## Lab 4 — DMA Transfers

### Goal

Move large messages using **DMA** to reduce CPU overhead.

### Steps

1. Allocate DMA-capable memory with `dma_alloc_coherent()` or streaming DMA APIs.
2. Implement a kmsgpipe_dma_transfer() helper that:
   - Prepares DMA descriptors
   - Submits to the DMA engine
   - Waits on completion callbacks
3. Add sysfs entries to toggle DMA mode on/off.
4. Compare throughput vs. non-DMA mode.

### Key APIs

- `dma_alloc_coherent()`, `dma_free_coherent()`
- `dma_map_single()`, `dma_unmap_single()`
- `dmaengine_prep_slave_single()`, `dma_async_issue_pending()`

### Test

```bash
echo dma=1 > /sys/class/kmsgpipe/mode
dd if=/dev/zero of=/dev/kmsgpipe bs=4K count=100

```

## Lab 5 — Device Tree Integration

### Goal

Bind kmsgpipe to hardware using the Device Tree.

### Steps

1. Write a sample overlay entry:

```dts
kmsgpipe@4000 {
    compatible = "edu,kmsgpipe";
    reg = <0x4000 0x100>;
    interrupts = <15>;
    dma-channels = <1>;
};
```

2. Add of`_match_table` to your `platform_driver`.
3. Use `of_property_read_u32()` to parse custom properties.
4. Validate probe sequence via dmesg.

### Key APIs

- `of_match_ptr()`, `of_property_read_*()`
- `of_device_get_match_data()`
- `MODULE_DEVICE_TABLE(of, kmsgpipe_of_match)`

## Lab 6 — Integration Test & Profiling

### Goal

Perform end-to-end validation of DMA, interrupts, and platform behavior.

### Steps

1. Write a test script that:

   - Loads driver
   - Configures DMA/sysfs attributes
   - Performs large read/write
   - Checks interrupt counters

2. Profile performance using `perf` or `ftrace`.

### Example

```bash
sudo perf record -e irq:irq_handler_entry,irq:irq_handler_exit dd if=/dev/zero of=/dev/kmsgpipe bs=4K count=100
sudo perf report
```

---

## Optional Challenge — Hybrid Firmware Link

If using a Micro:bit V2 or STM32 Discovery board:

- Implement a small firmware that echoes messages or generates events (UART/SPI).
- Expose it as a /dev/kmsgpipe_hw endpoint.
- Communicate via your Linux driver over USB CDC, UART, or SPI.
  This bridges `embedded firmware` and `host driver development`.

---

## Deliverables

| Item                | Description                                                   |
| ------------------- | ------------------------------------------------------------- |
| Source Code         | Hardware-integrated `kmsgpipe` supporting interrupts, DMA, DT |
| Firmware (optional) | Microcontroller firmware for test platform                    |
| Documentation       | Updated module usage, DT overlay, sysfs guide                 |
| Performance Report  | Comparison between DMA/non-DMA and interrupt polling modes    |

## Expected Outcome

By completing Part 3, you’ll have a driver that:

- integrates cleanly with Linux’s hardware device model
- supports efficient and interrupt-driven I/O
- demonstrates DMA and Device Tree binding
- is portable across x86 emulation and ARM boards

You will be fully prepared for Part 4, focusing on **testing**, **instrumentation**, **and Rust porting of selected driver components**.

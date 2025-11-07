# 🧱 Phase 1: Hardware-Independent Driver Labs

These build your foundation — the kernel concepts that apply to any device driver.

Each topic will have **3 lab tasks**:

- Minimal driver for concept understanding
- Feature or concurrency extension
- Debug/analysis or optimization version

| Topic                                      | Lab 1                                          | Lab 2                                                 | Lab 3                                       |
| ------------------------------------------ | ---------------------------------------------- | ----------------------------------------------------- | ------------------------------------------- |
| **1️⃣ Character Device Basics**             | Create `/dev/demo` that echoes user input      | Add custom `ioctl()` for clearing buffer              | Support multiple devices sharing same ops   |
| **2️⃣ Memory Management in Drivers**        | Use `kmalloc`/`kfree` safely                   | Switch to `vmalloc` and measure difference            | Expose kernel buffer via `mmap()`           |
| **3️⃣ Concurrency & Synchronization**       | Two writers cause race condition intentionally | Protect with `spinlock`                               | Replace with `mutex` and add wait queue     |
| **4️⃣ Workqueues, Tasklets, Timers**        | Use timer to print heartbeat                   | Switch to tasklet for deferred work                   | Replace with workqueue and cancel on unload |
| **5️⃣ Interrupt Handling**                  | Dummy interrupt simulation (via `raise_irq`)   | Use threaded IRQ                                      | Measure latency with `ktime_get`            |
| **6️⃣ Blocking & Async I/O**                | Implement blocking `read()`                    | Add `poll()` support                                  | Add async notification via `fasync_helper`  |
| **7️⃣ Kernel Threads & Deferred Execution** | Launch a kernel thread printing counter        | Convert to stoppable thread via `kthread_should_stop` | Replace with workqueue variant              |

# ⚙️ Phase 2: Hardware-Dependent / Bus-Oriented Driver Labs

Now you touch real buses and understand how drivers interact with hardware controllers.

| Topic                                 | Lab 1                                               | Lab 2                                   | Lab 3                                  |
| ------------------------------------- | --------------------------------------------------- | --------------------------------------- | -------------------------------------- |
| **1️⃣ Platform Drivers & Device Tree** | Register dummy platform device and driver           | Bind via Device Tree overlay            | Expose sysfs attributes for control    |
| **2️⃣ I²C Driver**                     | Simulate I²C client device (dummy EEPROM)           | Communicate with actual sensor on STM32 | Add interrupt-driven transfer          |
| **3️⃣ SPI Driver**                     | Write loopback SPI test driver                      | Connect to external ADC or display      | Add DMA for SPI transfer               |
| **4️⃣ UART Driver**                    | Write pseudo-TTY driver                             | Connect to MCU UART (micro:bit/STM32)   | Add interrupt-driven RX/TX             |
| **5️⃣ USB Driver**                     | Write simple USB skeleton driver (usb_skel.c clone) | Enumerate USB HID device                | Send control/bulk transfer to firmware |
| **6️⃣ DMA (optional)**                 | Use DMA engine API for memory copy                  | Add scatter-gather list                 | Measure latency vs CPU copy            |

# 🔌 Phase 3: Firmware Integration Projects (Your Dev Boards)

Once you have the building blocks, we connect your Linux driver to firmware running on micro:bit V2 or STM32 Discovery.
Each board acts as a smart peripheral controlled by your driver.

| Project                       | Dev Board       | Description                                                      | Linux Driver Concept Reinforced |
| ----------------------------- | --------------- | ---------------------------------------------------------------- | ------------------------------- |
| **1️⃣ GPIO Controller**        | micro:bit       | Expose micro:bit pins as GPIOs controllable via `/dev/microgpio` | Platform driver, I²C            |
| **2️⃣ Sensor Data Stream**     | STM32 Discovery | Stream accelerometer data via UART or SPI                        | Char driver + blocking I/O      |
| **3️⃣ Custom USB Gadget**      | STM32           | Act as USB device streaming data to PC                           | USB driver, async I/O           |
| **4️⃣ Display / LED Control**  | micro:bit       | Write to 5×5 LED matrix via I²C                                  | I²C + sysfs interface           |
| **5️⃣ Remote Firmware Update** | STM32           | Send firmware image over SPI/I²C from Linux host                 | SPI + DMA + file I/O            |

# 🧩 Suggested Folder Layout

```bash
kernel-driver-lab/
 ├── 00_setup_qemu/
 ├── 01_char_driver/
 ├── 02_memory_management/
 ├── 03_concurrency/
 ├── 04_workqueues/
 ├── 05_interrupts/
 ├── 06_blocking_io/
 ├── 07_kthreads/
 ├── 08_platform_driver/
 ├── 09_i2c_driver/
 ├── 10_spi_driver/
 ├── 11_uart_driver/
 ├── 12_usb_driver/
 ├── 13_dma/
 ├── 14_microbit_project/
 ├── 15_stm32_project/
 └── README.md
```

Each directory will include:

- `Makefile`
- `driver.c`
- `test_user.c` (if needed)
- `NOTES.md` (learning notes + outputs)
- optional `firmware/` folder (for STM32 or micro:bit)

## 🧠 Learning Notes per Lab

Each NOTES.md will include:

- Concepts reinforced
- Kernel APIs used
- Common pitfalls or kernel oops seen
- Reflection: “What I learned and why it matters”

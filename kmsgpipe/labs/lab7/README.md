## ✅ Lab 7 – Interrupt Handling (Pure Software Version)

You don’t need real hardware to learn interrupts.

### Use a Kernel Timer to Simulate Hardware Interrupt

- Create a kernel timer (struct timer_list)

- When timer fires → call your "ISR"

- Inside ISR:
  - Acquire spinlock

  - Push message into ring buffer

  - Wake up waiting readers

- Re-arm timer

**This simulates:**

> “Hardware generates data periodically via interrupt”

**Important learning goals:**

- Use spin_lock_irqsave
- Understand interrupt context (cannot sleep)
- Wake up waitqueue from interrupt
- Protect shared buffer

This integrates perfectly with your ring buffer.

### What Lab 7 Should Teach You

You must deliberately enforce:

- ❌ No sleeping in ISR
- ❌ No mutex in ISR
- ✅ Only spinlocks
- ✅ Proper context checking

You should intentionally:

- Try using mutex in ISR → observe warning
- Print in_interrupt() and in_atomic() values
- Demonstrate race without spinlock

> Make this lab about context discipline.

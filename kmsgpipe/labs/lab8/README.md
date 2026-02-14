## ✅ Lab 8 – Platform Device + Probe/Remove (Pure Software)

This is 100% possible without hardware.

You can create:

- A fake platform device
- A matching platform driver
- Register device in module_init
- Let kernel call probe
- Use your ring buffer inside probe
- Clean up in remove

This teaches:

- Driver binding model
- Device lifecycle
- Resource management
- devm\_\* helpers

### How To Integrate With kmsgpipe

Instead of manually creating device node in init:
Move that into:

```c
probe()
```

And cleanup into:

```c
remove()
```

Now your driver becomes a proper hardware-style driver.

### Bonus: Simulate Memory-Mapped Registers

Inside probe:

- Allocate memory via devm_kzalloc
- Pretend it's hardware registers
- Access it using wrapper functions

This helps prepare for ioremap() later.

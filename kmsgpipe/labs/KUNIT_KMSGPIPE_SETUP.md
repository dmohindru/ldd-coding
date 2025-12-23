# KUnit Setup & Execution for Out-of-Tree Linux Driver (kmsgpipe)

This document is a concise, repeatable recipe for setting up and running **KUnit tests**
for an *out-of-tree* Linux kernel driver, based on the `kmsgpipe` driver.

---

## 1. Big Picture (Mental Model)

KUnit testing involves **three layers**:

1. **Driver Kconfig**
2. **Driver Makefile (Kbuild)**
3. **KUnit runner (kunit.py)**

KUnit builds a special kernel, boots it (QEMU/UML), and parses test output.

Golden rule:
> If code cannot be tested with KUnit, it is probably doing too much.

---

## 2. Driver Layout (Working)

```
drivers/misc/kmsgpipe/
├── Kconfig
├── Makefile
├── kmsgpipe_core.c
├── kmsgpipe_core.h
├── kmsgpipe_core_test.c
├── kmsgpipe_fops.c
├── kmsgpipe_module.c
└── kunit/
    └── kmsgpipe.config
```

---

## 3. Driver Kconfig

```kconfig
menu "Kmsgpipe driver"

config KMSGPIPE
    tristate "Kmsgpipe character driver"
    default n

config KMSGPIPE_KUNIT_TEST
    bool "Kmsgpipe KUnit tests"
    depends on KUNIT && KMSGPIPE
    default n

endmenu
```

---

## 4. Driver Makefile (Kbuild)

```make
obj-$(CONFIG_KMSGPIPE) += kmsgpipe_global.o

kmsgpipe_global-objs :=     kmsgpipe_module.o     kmsgpipe_fops.o     kmsgpipe_core.o

obj-$(CONFIG_KMSGPIPE_KUNIT_TEST) += kmsgpipe_core_test.o
```

---

## 5. KUnit Config File

Location:
```
drivers/misc/kmsgpipe/kunit/kmsgpipe.config
```

Contents:
```text
CONFIG_KUNIT=y
CONFIG_KMSGPIPE=y
CONFIG_KMSGPIPE_KUNIT_TEST=y
```

---

## 6. Running Tests (Canonical Command)

From kernel source root:

```bash
./tools/testing/kunit/kunit.py run   --arch=x86_64   --kunitconfig=drivers/misc/kmsgpipe/kunit/kmsgpipe.config
```

---

## 7. Required Host Dependencies

```bash
sudo apt install flex bison qemu-system-x86
```

---

## 8. Common Pitfalls

- `--kconfig_add` does not accept newline-separated strings
- Driver must live inside kernel tree (or be symlinked)
- Test code must not touch hardware or user space

---

## 9. Result

- Kernel built
- Driver built
- KUnit tests executed
- Output parsed successfully

---

## 10. Status

✔ KUnit kernel boots  
✔ Tests discovered  
✔ Tests passed  

You now have a **repeatable KUnit workflow**.

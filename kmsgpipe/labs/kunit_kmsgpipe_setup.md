# KUnit Setup & Execution for Out-of-Tree Linux Driver (kmsgpipe)

This document is a concise, repeatable recipe for setting up and running **KUnit tests**
for an _out-of-tree_ Linux kernel driver, based on the `kmsgpipe` driver.

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

## Out of tree kunit testing technique

### 1. Create a symbolic link of your source directory in kernel directory at location `driver/misc/`

```shell
cd <Repo-Root>/kernel/drivers/misc
ln -s ../../../labs/lab1/driver kmsgpipe_lab<n>
```

### 2. Edit Kconfig file located at `driver/misc/Kconfig` and add following line at the end

```text
source "drivers/misc/kmsgpipe_lab<n>/Kconfig"
```

### 3. Modify Makefile located at `driver/misc/Makefile` and add following line at the end

```text
obj-$(CONFIG_KMSGPIPE_LAB<N>) += kmsgpipe_lab<n>/
```

Note:

- CONFIG_KMSGPIPE_LAB<N> should have been defined in your driver `Kconfig` file
- kmsgpipe_lab<n> is a folder created in kernel directory in step 1

### 4. Run Tests with following commands

```shell
cd repo_root/kernel

./tools/testing/kunit/kunit.py run \
  --arch=x86_64 \
  --kunitconfig=drivers/misc/kmsgpipe_lab1/kunit/driver.config
```

or run the helper script in driver directory

```shell
./run_kunit.sh
```

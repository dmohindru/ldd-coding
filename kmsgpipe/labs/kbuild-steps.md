# How `module.mk` Builds a Linux Kernel Module (Beginner-Friendly)

This document explains, in plain English, what happens when you run:

```bash
make -f module.mk
```

and how this builds a Linux kernel driver as a loadable module (`.ko`).

---

## What is this file for?

The file `module.mk` is **not** a normal Makefile that compiles code by
itself.

Instead, its job is to **ask the Linux kernel's own build system** to
compile your driver code correctly.

This is the _official_ and _recommended_ way to build Linux kernel
modules.

---

## Step-by-step: what actually happens

### Step 1: Find the kernel build system

```make
KDIR ?= /lib/modules/$(shell uname -r)/build
```

Plain English:

> "Find the build files for the Linux kernel I am currently running."

- `uname -r` prints your kernel version
- `/lib/modules/<version>/build` points to the kernel build directory
- This directory contains the kernel's Makefiles and configuration

Now Make knows _which kernel_ it is building against.

---

### Step 2: Remember where the driver source code is

```make
PWD := $(shell pwd)
```

Plain English:

> "Remember the directory I am currently in."

This directory contains: - Your driver `.c` files - Your driver
`Makefile`

---

### Step 3: You run `make`

```bash
make -f module.mk
```

Plain English:

> "Use `module.mk` instead of the default `Makefile`."

Make looks for the rule named `all`.

---

### Step 4: Jump into the kernel build system

```make
$(MAKE) -C "$(KDIR)" M="$(PWD)" modules
```

This line does the real work.

Plain English:

> "Go to the kernel build directory and ask the kernel to build my
> driver."

Broken down:

- `$(MAKE)` → run make again
- `-C "$(KDIR)"` → change directory to the kernel build system
- `M="$(PWD)"` → tell the kernel where the driver source lives
- `modules` → tell the kernel to build external modules

Effectively, this happens:

```bash
cd /lib/modules/<kernel-version>/build
make M=/path/to/driver modules
```

---

### Step 5: Kernel reads your driver Makefile

Inside the kernel build system:

1.  The kernel enters your driver directory
2.  It reads your driver's `Makefile`
3.  It looks for lines like:

```make
obj-m += my_driver.o
```

Plain English:

> "This object should become a kernel module."

---

### Step 6: Kernel builds the module

The kernel now:

1.  Compiles `.c` files into `.o` files
2.  Links them into a `.ko` file
3.  Generates helper metadata files

Result:

```text
my_driver.ko
```

This `.ko` file can be loaded using `insmod` or `modprobe`.

---

### Step 7: Cleaning build files

When you run:

```bash
make -f module.mk clean
```

The kernel build system removes:

- `.o` files
- `.ko` files
- `.mod.c` files
- `.symvers`
- `.cmd` files

This keeps your source directory clean.

---

## One-sentence summary

> "This Makefile asks the Linux kernel's build system to compile my
> driver code and turn it into a loadable kernel module."

---

## Mental picture

```text
You
 │
 │ make -f module.mk
 ▼
Kernel build system
 │
 │ reads your driver Makefile
 ▼
Builds .ko module
```

Your Makefile is just a **messenger**, not the builder.

---

## Why this method is used

- Ensures compatibility with the running kernel
- Uses correct compiler flags automatically
- Handles kernel-specific build rules
- Matches how real kernel drivers are built

---

## TL;DR

- `module.mk` does not compile code itself
- It hands control to the kernel build system
- The kernel build system builds your driver
- Output is a `.ko` kernel module

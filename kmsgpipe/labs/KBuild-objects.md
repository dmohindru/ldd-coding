## This file explains what is the purpose of different build object

- obj-y
- obj-m
- obj-n

### 1. What these variables really are

In kernel Makefiles (Kbuild), these are lists of object files:

| Variable | Meaning                             |
| -------- | ----------------------------------- |
| `obj-y`  | Built **into the kernel image**     |
| `obj-m`  | Built as **loadable modules (.ko)** |
| `obj-n`  | **Not built at all**                |

They are not special syntax, just ordinary Make variables with special meaning to Kbuild.

### 2. Where do these values come from?

From Kconfig → .config.

Example:

```text
CONFIG_KMSGPIPE=y
```

Kbuild expands this:

```make
obj-$(CONFIG_KMSGPIPE) += kmsgpipe_global.o
```

Becomes:

```make
obj-y += kmsgpipe_global.o
```

If instead:

```text
CONFIG_KMSGPIPE=m
```

Then:

```make
obj-m += kmsgpipe_global.o
```

If disabled:

```text
CONFIG_KMSGPIPE is not set
```

Then:

```make
obj-n += kmsgpipe_global.o   # ignored
```

👉 This is the key pattern.

### 3.

Why three states exist (y, m, n)

**y – built-in**

- Code is compiled into vmlinux
- Always present
- No insmod
- Available early at boot

Used for:

- core kernel features
- filesystems
- security primitives

**m – module**

- Built as .ko
- Loaded with insmod / modprobe
- Can be unloaded

Used for:

- drivers
- optional subsystems
- experimentation

**n – not built**

- Code is ignored
- No compilation
- Zero cost

Used when:

- feature not needed
- dependencies not met

## 4 What does obj-m := kmsgpipe_global.o mean?

This line:

```make
obj-m := kmsgpipe_global.o
```

Means:

> “Always build this as a module.”

It bypasses Kconfig.

That’s why this form is typically used for:

- out-of-tree modules
- quick experiments
- tutorials

But in-tree code prefers:

```make
obj-$(CONFIG_KMSGPIPE) += kmsgpipe_global.o
```

## 5. What about composite objects (\*-objs)?

This is where things often get confusing.

Example:

```make
obj-$(CONFIG_KMSGPIPE) += kmsgpipe_global.o

kmsgpipe_global-objs := \
    kmsgpipe_module.o \
    kmsgpipe_fops.o \
    kmsgpipe_core.o
```

Interpretation:

- Build one logical object: kmsgpipe_global.o
- That object is composed of multiple .o files
- Kbuild links them together

Think:

```text
kmsgpipe_global.o
├── kmsgpipe_module.o
├── kmsgpipe_fops.o
└── kmsgpipe_core.o
```

## 6. Why split objects this way?

Because it lets you:

- Share code (kmsgpipe_core.o)
- Test code (kmsgpipe_core_test.o)
- Avoid pulling test code into production builds
- Keep file-level responsibilities clean

This is exactly why KUnit works well with this pattern.

## 7. What happens with KUnit test objects?

Example:

```make
obj-$(CONFIG_KMSGPIPE_KUNIT_TEST) += kmsgpipe_core_test.o
```

If enabled:

- kmsgpipe_core_test.o is compiled
- It is linked into the kernel
- Tests auto-register and run at boot

If disabled:

- Test file is ignored
- Zero overhead

## 8. Important subtlety: where these objects go

| Variable | Output     |
| -------- | ---------- |
| `obj-y`  | `vmlinux`  |
| `obj-m`  | `.ko` file |
| `obj-n`  | nowhere    |

So if:

```make
obj-y += foo.o
```

That code cannot be unloaded.

## 9. Why kernel Makefiles look “weird”

Because they are **declarative**, not procedural.

You are not saying:

> “Compile this, then link that.”

You are saying:

> “If feature X is enabled, these objects belong to the kernel.”

Kbuild handles:

- compilation
- ordering
- linking
- dependency resolution

## 10. One-sentence takeaway (lock this in)

> **`obj-y` builds-in, `obj-m` modularizes, `obj-n` ignores — and Kconfig decides which one you get.**

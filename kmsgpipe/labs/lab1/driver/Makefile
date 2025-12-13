# Makefile to build kmsgpipe_basic kernel module
#
# Usage:
#   make                # build the module (uses KDIR default)
#   make KDIR=/path/to/kernel-headers   # override kernel headers/build dir
#   make clean
#   sudo make insmod    # build then insert module
#   sudo make rmmod     # remove module
#   make modinfo        # show module info

# Module object (name must match kmsgpipe_basic.c -> kmsgpipe_basic.o)
obj-m := kmsgpipe_basic.o

# Default kernel build dir (can be overridden on command line)
KDIR ?= /usr/src/linux-headers-6.14.0-35-generic

# Where we are building from
PWD := $(shell pwd)

# Extra include paths you provided (passed to the kernel build as EXTRA_CFLAGS)
EXTRA_INCLUDES := \
    -I/usr/src/linux-headers-6.14.0-35-generic/include \
    -I/usr/src/linux-headers-6.14.0-35-generic/include/uapi \
    -I/usr/src/linux-headers-6.14.0-35-generic/include/generated \
    -I/usr/src/linux-headers-6.14.0-35-generic/arch/x86/include \
    -I/usr/src/linux-headers-6.14.0-35-generic/arch/x86/include/uapi \
    -I/usr/src/linux-headers-6.14.0-35-generic/arch/x86/include/generated

# You can also pass custom CFLAGS:
# make EXTRA_CFLAGS="-O0 -g -Wno-unused"   (overrides here)
EXTRA_CFLAGS ?= $(EXTRA_INCLUDES)

.PHONY: all clean insmod rmmod modinfo

all:
    $(MAKE) -C "$(KDIR)" M="$(PWD)" modules EXTRA_CFLAGS="$(EXTRA_CFLAGS)"

clean:
    $(MAKE) -C "$(KDIR)" M="$(PWD)" clean

insmod: all
    sudo insmod ./kmsgpipe_basic.ko

rmmod:
    -@sudo rmmod kmsgpipe_basic || true

modinfo:
    modinfo ./kmsgpipe_basic.ko || true
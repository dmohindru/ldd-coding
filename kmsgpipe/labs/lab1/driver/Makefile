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

# Default kernel build dir: default to the running kernel's build tree so
# `make` works without needing to specify KDIR explicitly. You can still
# override it on the command line, e.g. `make KDIR=/path/to/headers`.
KDIR ?= /lib/modules/$(shell uname -r)/build

# Where we are building from
PWD := $(shell pwd)

# Directory to collect build artifacts
BUILD_DIR := $(PWD)/build

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
	@mkdir -p "$(BUILD_DIR)"
	@mv -f *.ko *.mod.c *.mod.o *.mod.* *.o *.symvers *.d *.cmd *.order "$(BUILD_DIR)" 2>/dev/null || true


clean:
	$(MAKE) -C "$(KDIR)" M="$(PWD)" clean
	@rm -rf "$(BUILD_DIR)"

insmod: all
	sudo insmod "$(BUILD_DIR)/kmsgpipe_basic.ko"

rmmod:
	-@sudo rmmod kmsgpipe_basic || true

modinfo:
	modinfo "$(BUILD_DIR)/kmsgpipe_basic.ko" || true
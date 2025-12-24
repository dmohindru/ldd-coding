# Main make file used for building loadable kernel modules
obj-m := kmsgpipe_global.o
kmsgpipe_global-objs := kmsgpipe_module.o kmsgpipe_fops.o kmsgpipe_core.o

KDIR ?= /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

BUILD_DIR := $(PWD)/build
KMSGPIPE_GLOBAL := "kmsgpipe_global"

EXTRA_INCLUDES := \
    -I/usr/src/linux-headers-6.14.0-35-generic/include \
    -I/usr/src/linux-headers-6.14.0-35-generic/include/uapi \
    -I/usr/src/linux-headers-6.14.0-35-generic/include/generated \
    -I/usr/src/linux-headers-6.14.0-35-generic/arch/x86/include \
    -I/usr/src/linux-headers-6.14.0-35-generic/arch/x86/include/uapi \
    -I/usr/src/linux-headers-6.14.0-35-generic/arch/x86/include/generated


EXTRA_CFLAGS ?= $(EXTRA_INCLUDES)

.PHONY: all clean insmod rmmod modinfo

all:
	$(MAKE) -C "$(KDIR)" M="$(PWD)" modules EXTRA_CFLAGS="$(EXTRA_CFLAGS)"
	@mkdir -p "$(BUILD_DIR)"
	# move common build artifacts (including some dot-prefixed command files)
	@mv -f \
		*.ko \
		*.mod.c \
		*.mod.o \
		*.mod.* \
		*.mod \
		*.o \
		*.symvers \
		*.d \
		*.cmd \
		*.order \
		.kmsgpipe_global* \
		.*.cmd \
		.*module* \
		"$(BUILD_DIR)" 2>/dev/null || true


clean:
	$(MAKE) -C "$(KDIR)" M="$(PWD)" clean
	@rm -rf "$(BUILD_DIR)"

insmod-global:
	sudo insmod "$(BUILD_DIR)/$(KMSGPIPE_GLOBAL).ko"

rmmod-global:
	-@sudo rmmod $(KMSGPIPE_GLOBAL) || true

modinfo-global:
	modinfo "$(BUILD_DIR)/$(KMSGPIPE_GLOBAL).ko" || true

.PHONY: devnode-global rmdev-global
devnode-global:
	@MAJOR=$$(awk '/kmsgpipe_global/ {print $$1}' /proc/devices); \
	if [ -z "$$MAJOR" ]; then \
		echo "Device not registered; load module first (make insmod)"; \
		exit 1; \
	fi; \
	sudo mknod /dev/$(KMSGPIPE_GLOBAL) c $$MAJOR 0 || true; \
	sudo chmod 0666 /dev/$(KMSGPIPE_GLOBAL) || true; \
	echo "/dev/$(KMSGPIPE_GLOBAL) created (major=$$MAJOR, minor=0)"

rmdev-global:
	-@sudo rm -f /dev/$(KMSGPIPE_GLOBAL) || true
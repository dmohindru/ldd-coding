# Header file location inclusion
ccflags-y += -I$(src)/../../include

# External module build
obj-m += kmsgpipe_lab2.o

# In-kernel / KUnit build
obj-$(CONFIG_KMSGPIPE_LAB2) += kmsgpipe_lab2.o

kmsgpipe_lab2-objs := \
	kmsgpipe_module.o \
	kmsgpipe_fops.o  \
	../../lib/src/kmsgpipe.o
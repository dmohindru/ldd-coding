# Header file location inclusion
ccflags-y += -I$(src)/../../include

# External module build
obj-m += kmsgpipe_lab5.o

# In-kernel / KUnit build
obj-$(CONFIG_KMSGPIPE_LAB5) += kmsgpipe_lab4.o

kmsgpipe_lab5-objs := \
	kmsgpipe_module.o \
	kmsgpipe_fops.o  \
	../../lib/src/kmsgpipe.o
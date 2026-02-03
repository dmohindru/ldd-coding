# Header file location inclusion
ccflags-y += -I$(src)/../../include

# External module build
obj-m += kmsgpipe_lab3.o

# In-kernel / KUnit build
obj-$(CONFIG_KMSGPIPE_LAB3) += kmsgpipe_lab3.o

kmsgpipe_lab3-objs := \
	kmsgpipe_module.o \
	kmsgpipe_fops.o  \
	../../lib/src/kmsgpipe.o
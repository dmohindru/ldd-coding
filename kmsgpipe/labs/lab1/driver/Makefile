# Kbuild Makefile for kmsgpipe used for kunit testing

obj-$(CONFIG_KMSGPIPE_LAB1) += kmsgpipe_global.o

kmsgpipe_global-objs := \
	kmsgpipe_module.o \
	kmsgpipe_fops.o \
	kmsgpipe_core.o

# Build KUnit tests only when enabled
obj-$(CONFIG_KMSGPIPE_LAB1_KUNIT_TEST) += kmsgpipe_core_test.o

# Common core objects
kmsgpipe_core-objs := kmsgpipe_core.o

# External module build
obj-m += kmsgpipe_global.o

# In-kernel / KUnit build
obj-$(CONFIG_KMSGPIPE_LAB1) += kmsgpipe_global.o

kmsgpipe_global-objs := \
	kmsgpipe_module.o \
	kmsgpipe_fops.o \
	kmsgpipe_core.o

# KUnit tests (reuse core code)
obj-$(CONFIG_KMSGPIPE_LAB1_KUNIT_TEST) += kmsgpipe_core_test.o
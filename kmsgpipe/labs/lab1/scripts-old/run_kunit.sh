#!/bin/bash
KERNEL=/home/dhruv/Programming/linux-device-driver-programming/ldd3e/ldd3-coding/kmsgpipe/kernel
# Change Me only for each new driver
DRIVER_DIR=kmsgpipe_lab1

$KERNEL/tools/testing/kunit/kunit.py run \
    --arch=x86_64 \
    --kunitconfig $KERNEL/drivers/misc/$DRIVER_DIR/kunit/driver.config

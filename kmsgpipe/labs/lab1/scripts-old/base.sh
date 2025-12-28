#!/bin/bash
set -u  # error on undefined variables

# Resolve driver root (scripts/..)
DRIVER_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Driver/module name (change once per driver)
MODULE_NAME="kmsgpipe_global"

# Build artifacts directory
BUILD_DIR="$DRIVER_DIR/build"

# Kernel build directory (external module build)
KERNEL_BUILD_DIR="/lib/modules/$(uname -r)/build"

# Makefiles
KBUILD_MAKEFILE="$DRIVER_DIR/Makefile"
MODULE_MAKEFILE="$DRIVER_DIR/module.mk"

# Device node info
DEVICE_NAME="$MODULE_NAME"
DEVICE_PATH="/dev/$DEVICE_NAME"

export DRIVER_DIR MODULE_NAME BUILD_DIR KERNEL_BUILD_DIR

#!/bin/bash
set -u

# Resolve driver root (scripts is symlink-safe)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DRIVER_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# Load per-driver config
if [[ ! -f "$DRIVER_DIR/driver.conf" ]]; then
    echo "ERROR: driver.conf not found in $DRIVER_DIR"
    exit 1
fi

source "$DRIVER_DIR/driver.conf"

BUILD_DIR="$DRIVER_DIR/build"
KERNEL_BUILD_DIR="/lib/modules/$(uname -r)/build"

export DRIVER_DIR MODULE_NAME BUILD_DIR KERNEL_BUILD_DIR DEVICE_NAME
echo "DRIVER_DIR: $DRIVER_DIR"
echo "MODULE_NAME: $MODULE_NAME"
echo "BUILD_DIR: $BUILD_DIR"
echo "KERNEL_BUILD_DIR: $KERNEL_BUILD_DIR"
echo "DEVICE_NAME: $DEVICE_NAME"


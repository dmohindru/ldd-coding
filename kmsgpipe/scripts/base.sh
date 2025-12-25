#!/bin/bash
set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DRIVER_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

CONF_FILE="$DRIVER_DIR/driver.conf"
if [[ ! -f "$CONF_FILE" ]]; then
    echo "ERROR: driver.conf not found in $DRIVER_DIR"
    exit 1
fi

source "$CONF_FILE"

# ---- Required variables ----
: "${MODULE_NAME:?MODULE_NAME not set in driver.conf}"
: "${MODULE_MAKEFILE:?MODULE_MAKEFILE not set in driver.conf}"

BUILD_DIR="$DRIVER_DIR/build"
KERNEL_BUILD_DIR="/lib/modules/$(uname -r)/build"
MODULE_KO="$BUILD_DIR/${MODULE_NAME}.ko"

export DRIVER_DIR MODULE_NAME MODULE_MAKEFILE BUILD_DIR KERNEL_BUILD_DIR MODULE_KO

echo "DRIVER_DIR: $DRIVER_DIR"
echo "MODULE_NAME: $MODULE_NAME"
echo "MODULE_MAKEFILE: $MODULE_MAKEFILE"
echo "BUILD_DIR: $BUILD_DIR"
echo "KERNEL_BUILD_DIR: $KERNEL_BUILD_DIR"
echo "DEVICE_NAME: $DEVICE_NAME"
echo "MODULE_KO: $MODULE_KO"


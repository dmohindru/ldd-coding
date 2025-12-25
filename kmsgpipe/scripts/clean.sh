#!/bin/bash
set -euo pipefail

source "$(dirname "$0")/base.sh"

echo "[clean] Cleaning module: $MODULE_NAME"
echo "[clean] Driver dir: $DRIVER_DIR"

# Run kernel build system clean
if [[ -f "$DRIVER_DIR/$MODULE_MAKEFILE" ]]; then
    echo "[clean] Running kbuild clean"
    make -f "$MODULE_MAKEFILE" clean || true
else
    echo "[clean] WARNING: $MODULE_MAKEFILE not found, skipping kbuild clean"
fi

# Remove local build directory
if [[ -d "$BUILD_DIR" ]]; then
    echo "[clean] Removing build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
else
    echo "[clean] No build directory to remove"
fi

echo "[clean] Done"

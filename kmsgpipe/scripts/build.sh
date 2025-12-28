#!/bin/bash
set -euo pipefail

source "$(dirname "$0")/base.sh"

echo "[build] Building module: $MODULE_NAME"
echo "[build] Driver dir: $DRIVER_DIR"
echo "[build] Using makefile: $MODULE_MAKEFILE"

mkdir -p "$BUILD_DIR"

BUILD_STATUS=0
if ! make -f "$MODULE_MAKEFILE"; then
    BUILD_STATUS=$?
fi

echo "[build] Build finished with status: $BUILD_STATUS"
echo "[build] Collecting artifacts..."

shopt -s nullglob dotglob

ARTIFACTS=(
    "$DRIVER_DIR"/*.ko
    "$DRIVER_DIR"/*.o
    "$DRIVER_DIR"/*.mod
    "$DRIVER_DIR"/*.mod.c
    "$DRIVER_DIR"/*.mod.o
    "$DRIVER_DIR"/*.mod.*
    "$DRIVER_DIR"/*.symvers
    "$DRIVER_DIR"/*.order
    "$DRIVER_DIR"/*.cmd
    "$DRIVER_DIR"/.*.cmd
)

for f in "${ARTIFACTS[@]}"; do
    mv -f "$f" "$BUILD_DIR/" 2>/dev/null || true
done

echo "[build] Artifacts moved to: $BUILD_DIR"

exit "$BUILD_STATUS"

#!/bin/bash
set -euo pipefail

source "$(dirname "$0")/base.sh"

DEVICE_NAME="${DEVICE_NAME:-$MODULE_NAME}"
DEVICE_PATH="/dev/$DEVICE_NAME"

echo "[insmod] Module  : $MODULE_NAME"
echo "[insmod] Device  : $DEVICE_PATH"
echo "[insmod] Driver  : $DRIVER_DIR"

# 1. Sanity check
if [[ ! -f "$MODULE_KO" ]]; then
    echo "[insmod] ERROR: Module not built: $MODULE_KO"
    echo "[insmod] Run ./scripts/build.sh first"
    exit 1
fi

# 2. Insert module
echo "[insmod] Inserting module..."
sudo insmod "$MODULE_KO" || {
    echo "[insmod] ERROR: insmod failed"
    exit 1
}

# 3. Extract major number from /proc/devices
echo "[insmod] Resolving major number..."
MAJOR=$(awk -v name="$DEVICE_NAME" '$2 == name {print $1}' /proc/devices)

if [[ -z "$MAJOR" ]]; then
    echo "[insmod] ERROR: Could not find device in /proc/devices"
    echo "[insmod] Did the module call register_chrdev()?"
    sudo rmmod "$MODULE_NAME" || true
    exit 1
fi

echo "[insmod] Major number: $MAJOR"

# 4. Create device node
if [[ -e "$DEVICE_PATH" ]]; then
    echo "[insmod] Device node already exists: $DEVICE_PATH"
else
    echo "[insmod] Creating device node..."
    sudo mknod "$DEVICE_PATH" c "$MAJOR" 0
    sudo chmod 0666 "$DEVICE_PATH"
fi

# 5. Final confirmation
echo "[insmod] SUCCESS ✅"
echo "[insmod] Device ready at: $DEVICE_PATH"
echo "[insmod] Usage example:"
echo "         echo hello > $DEVICE_PATH"

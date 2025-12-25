#!/bin/bash
set -euo pipefail

source "$(dirname "$0")/base.sh"

DEVICE_NAME="${DEVICE_NAME:-$MODULE_NAME}"
DEVICE_PATH="/dev/$DEVICE_NAME"

echo "[teardown] Module : $MODULE_NAME"
echo "[teardown] Device : $DEVICE_PATH"

# 1. Remove device node
if [[ -e "$DEVICE_PATH" ]]; then
    echo "[teardown] Removing device node: $DEVICE_PATH"
    sudo rm -f "$DEVICE_PATH"
else
    echo "[teardown] Device node not present (ok)"
fi

# 2. Remove kernel module
if lsmod | awk '{print $1}' | grep -q "^$MODULE_NAME$"; then
    echo "[teardown] Removing module..."
    sudo rmmod "$MODULE_NAME" || {
        echo "[teardown] ERROR: Failed to remove module"
        exit 1
    }
else
    echo "[teardown] Module not loaded (ok)"
fi

echo "[teardown] SUCCESS ✅ System cleaned"

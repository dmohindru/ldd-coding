#!/bin/sh

module="blocky"
device="blocky"
# invoke rmmod with all arguments we got
rmmod $module || exit 1

# Remove stale nodes

rm -f /dev/${device}
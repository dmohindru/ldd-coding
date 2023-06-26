#!/bin/sh

module="simplechar"
device="simplechar"
# invoke rmmod with all arguments we got
rmmod $module || exit 1

# Remove stale nodes

rm -f /dev/${device}
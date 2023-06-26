#!/bin/sh

module="simplechar"
device="simplechar"
mode="664"

# Group: since distributions do it differently, look for wheel or use staff
if grep -q '^staff:' /etc/group; then
    group="dhruv"
else
    group="dhruv"
fi

# invoke insmod with all arguments we got
# and use a pathname, as insmod doesn't look in . by default
insmod ./$module.ko || exit 1

# retrieve major number
major=$(awk "\$2==\"$module\" {print \$1}" /proc/devices)

rm -f /dev/${device}
echo "creating device file /dev/${device} with major number: ${major}, minor: 0"
mknod /dev/${device} c $major 0
chgrp $group /dev/${device}
chmod $mode  /dev/${device}
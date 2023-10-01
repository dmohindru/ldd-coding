#!/bin/sh

module="blocky"
device="blocky"
mode="664"
blocky_buffer_size="50"
# Group: since distributions do it differently, look for wheel or use staff
if grep -q '^staff:' /etc/group; then
    group="dhruv"
else
    group="dhruv"
fi

# If command line argument greater than one is provided use first argument as buffer size for blocky
if [ $# -ge 1 ]; then
  blocky_buffer_size=$1
fi

echo "Creating blocky device with buffer size of $blocky_buffer_size"
# invoke insmod with all arguments we got
# and use a pathname, as insmod doesn't look in . by default
insmod ./$module.ko buffer_size="$blocky_buffer_size" || exit 1

# retrieve major number
major=$(awk "\$2==\"$module\" {print \$1}" /proc/devices)

rm -f /dev/${device}
echo "creating device file /dev/${device} with major number: ${major}, minor: 0"
mknod /dev/${device} c $major 0
chgrp $group /dev/${device}
chmod $mode  /dev/${device}
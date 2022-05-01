#! /bin/sh

# This is a file to make a special file for scull device
# A special file allows read/write access to device
# To run the file: sudo sh scull_load.sh

module="scull"
device="scull"
mode="664"

/sbin/insmod ./$module.ko $* || exit 1

rm -f /dev/${device}[0-3]

major=$(awk "\$2==\"$module\" {print \$1}" /proc/devices)

mknod /dev/${device}0 c $major 0
mknod /dev/${device}1 c $major 1
mknod /dev/${device}2 c $major 2
mknod /dev/${device}3 c $major 3

#group="staff"
#grep -q '^staff:' /ect/group || group="wheel"

#chgrp $group /dev/${device}[0-3]
#chmode $mode /dev/${device}[0-3]
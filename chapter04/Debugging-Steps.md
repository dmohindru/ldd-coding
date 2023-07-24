### Step 1
Compile linux kernel with debugging support, setup GDB and run kernel in QEMU
Follow this [post](https://medium.com/@daeseok.youn/prepare-the-environment-for-developing-linux-kernel-with-qemu-c55e37ba8ade)

### Step 2
Compile linux kernel driver against download kernel source

### Step 3
Share files between host machine and virtual machine (QEMU)
```shell
qemu-system-x86_64 \
-virtfs local,path=/home/dhruv/Programming/linux-device-driver-programming/ldd3e/ldd3-coding/chapter03/simple_char_driver,mount_tag=hostshare,security_model=none \
-kernel arch/x86/boot/bzImage \
-boot c -m 2049M -hda ../buildroot/output/images/rootfs.ext4 \
-append "root=/dev/sda rw console=ttyS0,115200 acpi=off nokaslr" \
-serial stdio -display none
```

### Step 4
Load kernel module in virtual machine
Load kernel module symbol file in gdb
Set a break point at some code in kernel module 


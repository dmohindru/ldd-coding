### Introduction
<hr>
This wiki is about steps required to debug linux device driver using interactive debugging using gdb.<p>
I couldn't find any single post that cover all the steps required to make this happen. Steps in this wiki has been collected for various post some of them listed in Links section.<p>
For this wiki I have taken example of <b>simple_char_driver</b> listed in [chapter03](../chapter03/simple_char_driver) directory.

<br>This driver defines following function that we will use to set breakpoint for this exercise.
- simple_char_read
- simple_char_write
- simple_char_open
- simple_char_release

<br>This driver also has some global variable that we can use to print their value while debugging exercise.
- int simple_char_major
- int simple_char_minor
- struct simple_char_driver *char_driver_p


### Steps
<hr>

General outline of these steps
1. Initial preparation. Follow this [link](https://medium.com/@daeseok.youn/prepare-the-environment-for-developing-linux-kernel-with-qemu-c55e37ba8ade)
   1. Download linux kernel
   2. Compile kernel with debugging support
   3. Prepare file system to be used (Buildroot)
   4. Launch kernel with debug flags.
2. Share folder on host machine with virtual machine (qemu) running linux kernel with debug support. 
Idea behind this approach is that its easy to share compiled kernel module with virtual machine. 
<br>After following above two steps you should be ready with following command to launch linux in qemu
```shell
$ qemu-system-x86_64 -s \
-virtfs local,path=/home/dhruv/Programming/linux-device-driver-programming/ldd3e/ldd3-coding/chapter03/simple_char_driver,mount_tag=hostshare,security_model=none -kernel arch/x86/boot/bzImage \
-boot c -m 2049M -display none \
-append "root=/dev/sda rw console=ttyS0,115200 acpi=off nokaslr" \
-serial stdio \
-hda ../buildroot/output/images/rootfs.ext4
```
3. Launch gdb and connect with running linux instance
```shell
$ cd /path/to/top/of/linux/kernel/source
$ gdb ./vmlinuz
(gdb) target remote localhost:1234
(gdb) continue
```
4. Compile kernel module against downloaded kernel sources. For example check the [Makefile](../chapter03/simple_char_driver/Makefile) of <b>simple_char_driver</b>.
```shell
$ cd /to/your/driver/source/dir
$ make
```

5. Mount shared folder in qemu running linux
```shell
$ mount -t 9p -o trans=virtio hostshare /mnt
```

6. Run kernel driver loading script
```shell
$ cd /mnt
$ ./simple_char_load.sh
```

7. Get kernel module's various section load address from /sys file system. Make sure to copy address for .text, .data, .bss sections. We will need this when we load kernel module symbol file in gdb. 
```shell
$ cd /sys/module/simplechar/sections
$ ls -A1
$ cat .text .data .bss
0xffffffffa00f4000         (address of module's text section ...)
0xffffffffa00f4568         (... and data ...)
0xffffffffa00f47c0         (... and BSS) 
```

8. Load kernel module symbol file into running session of gdb
```shell
(gdb) add-symbol-file .../gdb1.ko 0xffffffffa00f4000 
 -s .data 0xffffffffa00f4568 
 -s .bss 0xffffffffa00f47c0
```

9. Set breakpoint in loaded kernel module and continue linux kernel execution.
```shell
(gdb) b simple_char_read
(gdb) b simple_char_write
(gdb) continue
```

10. Trigger kernel module execution. In qemu virtual machine  execute following command and kernel execution should be suspended at created break points.
```shell
$ echo "Hello World" > /dev/simplechar 
$ cat /dev/simplechar
```

### Links
<hr>
1. Compile linux kernel with debugging support, setup GDB and run kernel in QEMU. Follow this [post](https://medium.com/@daeseok.youn/prepare-the-environment-for-developing-linux-kernel-with-qemu-c55e37ba8ade)
<br>2. Loading debug symbols and setting break point in linux kernel module. [Link](https://www.linux.com/training-tutorials/kernel-newbie-corner-kernel-and-module-debugging-gdb/)



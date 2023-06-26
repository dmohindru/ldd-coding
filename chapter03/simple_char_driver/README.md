### Simple char driver
This is as simple as it can get implementation of char driver. This demonstrates how to write a simple character based 
driver and load it into running kernel  

### Learning objective
This driver demonstrates following
- driver init function
- driver exit function
- registering a char driver
- de-registering a char driver
- open() system call 
- close() system call 
- write() system call
- read() system call

### Driver working
- On every single write to its device file (/dev/simplechar) it releases previously held in memory data, allocated new memory for new incoming data and copies that data into driver's memory.
- On ever single read it copies data from driver kernel memory to user supplied buffer.

### Building, loading and running an example
Build a driver
```shell
make
```

clear previous build driver
```shell
make clean
```

Load driver in a kernel
```shell
sudo ./simple_char_load.sh
```

Unload driver from a kernel
```shell
sudo ./simple_char_unload.sh
```

Write to a device file
```shell
echo "Hello, World" > /dev/simplechar
```

Reading from a device file
```shell
cat /dev/simplechar
```
 
# Lab 2

This lab demonstrates how to use

- load a module with a parameter
- implement a debug FS
- Sleep and Wake process
  To demonstrate these features this lab will this [kmsgpipe ring buffer](../../lib/README.md) to implement its business logic

# Implementation

### Module parameters

Module will take the parameters on a commandline when loading a kernel module. And the parameter will decide how many entries should be there in kmsgpipe ring buffer.

### Ring Buffer consumers/producers

kmsgpipe will expose its functionality via device file. Following features will be implemented

- Process reading from the device file will act as a consumer of ring buffer and will get the data pointed at the head. If the buffer is empty the process is put to sleep till the buffer get a data from a writer process.
- Process writing from the device file will act as a producer of ring buffer and will write the data at the tail of buffer. If the buffer is full the process is put to sleep till the space in buffer is made available by a consumer process.

### Debug FS

This driver will implement a debug fs to show the current state of device with all the entries of the device ring buffer

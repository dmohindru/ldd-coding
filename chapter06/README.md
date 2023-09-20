### Device drivers operation covered in this chapter
- Device control via ioctl system call.

### [Calculator Driver](./calculator)
Purpose of this driver is to practice ioctl system calls. This driver has the following features/behaviour
- As name suggest it will perform calculation on two number. However, the second operand and arithmetic operation will be set by ioctl system calls.
- write operation will write the first operand to driver memory from user program and driver will do the calculation accordingly.
- read operation will provide result of last operation to user program.
- Only Admin user will be able to set the operation type (add, sub, mul, div) via ioctl system call.
- Normal user will be able to set the second operand for the calculation via ioctl.
- Normal user will be able to query device driver for current operation type and second operand via ioctl.
- Normal user will be able to reset the device driver to its original state (default value of operator and second operand)
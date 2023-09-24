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

### [Blocky Driver](./blocky)
Purpose of this driver is to experiment with blocking read/write system call. This driver has the following behaviour
- A driver will have user configurable size (default 50 bytes) buffer to store write/read data.
- User land program will try to write some data to this driver. If the amount of data being written is greater than size of driver buffer. The driver should return error.
- If a write request (bytes) is less than device buffer and request size is available in buffer write system call with complete with success.
- If a write request (bytes) is less than device buffer and request size is not available then driver should make calling process put to sleep.
- Similar behaviour need to be implemented for read system call request bytes need to be less than or equal to device buffer size and request data should be available.
- Whenever a write system call is competed successfully it should awake any sleeping read processes.
- Likewise on a successful read system call it should awake any sleeping write processes.
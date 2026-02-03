# lab 3

This lab will concentrate on

- ioctl calls
- privilege control

### IOCTL calls

Following parameters can be view/set

- Get KMSG_DATA_SIZE
- GET KMSG_CAPACITY
- GET KMSG_MSG_COUNT
- GET KMSG_WAITING_READERS
- GET KMSG_WAITING_WRITERS
- SET KMSG_EXPIRY_TIME (to be implemented in next lab)
- SET KMSG_CLEAR_ALL (privilege user only)

### PRIVILEGE CONTROL

- Message written by normal user can be read by any user (privilege and non-privilege)
- Message written by privilege user can only by read by privilege user, for other user return no access error.
- So effectively in a circular buffer if a message is written by privilege user, it can only be unblocked by privilege reader.

### Tips

use `get_user()` instead of `__get_user()` likewise do it for
`put_user()` instead of `__put_user()` as it does the `access_ok` check. And no need for
this code block

```c
if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok_wrapper(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =  !access_ok_wrapper(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err) return -EFAULT;
```

use this instead

```c
if (get_user(val, (int __user *)arg))
    return -EFAULT;
```

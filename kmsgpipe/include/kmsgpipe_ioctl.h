#ifndef KMSGPIPE_IOCTL_H
#define KMSGPIPE_IOCTL_H
#ifdef __KERNEL__
#include <linux/ioctl.h>
#else /* Userland */
#include <sys/ioctl.h>
#endif
#define KMSG_PIPE_IOC_MAGIC 'K' /* Hopefully it works */
/*
====== ===========================
    macro  parameters
====== ===========================
    _IO    none
    _IOW   write (read from userspace)
    _IOR   read (write to userpace)
    _IOWR  write and read
====== ===========================
*/

/*
 * S means "Set" through a ptr
 * G means "Get" through a ptr
 */
#define KMSGPIPE_IOC_G_DATA_SIZE _IOR(KMSG_PIPE_IOC_MAGIC, 1, int)
#define KMSGPIPE_IOC_G_CAPACITY _IOR(KMSG_PIPE_IOC_MAGIC, 2, int)
#define KMSGPIPE_IOC_G_MSG_COUNT _IOR(KMSG_PIPE_IOC_MAGIC, 3, int)
#define KMSGPIPE_IOC_G_READERS _IOR(KMSG_PIPE_IOC_MAGIC, 4, int)
#define KMSGPIPE_IOC_G_WRITERS _IOR(KMSG_PIPE_IOC_MAGIC, 5, int)
#define KMSGPIPE_IOC_S_EXPIRY_MS _IOW(KMSG_PIPE_IOC_MAGIC, 6, int)
#define KMSGPIPE_IOC_CLEAR _IO(KMSG_PIPE_IOC_MAGIC, 7)

#endif

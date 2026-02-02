#ifndef KMSGPIPE_IOCTL_H
#define KMSGPIPE_IOCTL_H
#ifdef __KERNEL__
#include <linux/ioctl.h>
#else /* Userland */
#include <sys/ioctl.h>
#endif
#define KMSGPIPE_IOC_MAGIC 'K' /* Hopefully it works */
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
#define KMSGPIPE_IOC_G_DATA_SIZE _IOR(KMSGPIPE_IOC_MAGIC, 1, long)
#define KMSGPIPE_IOC_G_CAPACITY _IOR(KMSGPIPE_IOC_MAGIC, 2, long)
#define KMSGPIPE_IOC_G_MSG_COUNT _IOR(KMSGPIPE_IOC_MAGIC, 3, long)
#define KMSGPIPE_IOC_G_READERS _IOR(KMSGPIPE_IOC_MAGIC, 4, long)
#define KMSGPIPE_IOC_G_WRITERS _IOR(KMSGPIPE_IOC_MAGIC, 5, long)
#define KMSGPIPE_IOC_S_EXPIRY_MS _IOW(KMSGPIPE_IOC_MAGIC, 6, long)
#define KMSGPIPE_IOC_CLEAR _IO(KMSGPIPE_IOC_MAGIC, 7)

#define KMSGPIPE_IOC_MAXNR 7

#endif

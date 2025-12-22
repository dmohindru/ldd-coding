#ifndef _KMSGPIPE_CORE_H_
#define _KMSGPIPE_CORE_H_

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/cdev.h>

#define KMSGPIPE_DEFAULT_BUFFER_SIZE 4096

struct kmsgpipe_char_driver
{
    void *data;
    unsigned long size;
    size_t data_len;
    struct mutex mutex;
    struct cdev char_dev;
};
typedef struct kmsgpipe_char_driver kmsgpipe_char_driver_t;

ssize_t kmsgpipe_write_core(struct kmsgpipe_char_driver *dev,
                            const char *src,
                            size_t count,
                            loff_t *pos);

ssize_t kmsgpipe_read_core(struct kmsgpipe_char_driver *dev,
                           char *dst,
                           size_t count,
                           loff_t *pos);

#endif

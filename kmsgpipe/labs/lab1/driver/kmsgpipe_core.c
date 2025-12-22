#include <linux/string.h>
#include <linux/errno.h>
#include "kmsgpipe_core.h"

ssize_t kmsgpipe_write_core(struct kmsgpipe_char_driver *dev,
                            const char *src,
                            size_t count,
                            loff_t *pos)
{
    if (!dev || !dev->data)
        return -ENODEV;

    if (count > dev->size)
        return -EFAULT;

    mutex_lock(&dev->mutex);

    memset(dev->data, 0, dev->size);
    memcpy(dev->data, src, count);
    dev->data_len = count;

    if (pos)
        *pos += count;

    mutex_unlock(&dev->mutex);
    return count;
}

ssize_t kmsgpipe_read_core(struct kmsgpipe_char_driver *dev,
                           char *dst,
                           size_t count,
                           loff_t *pos)
{
    size_t avail;

    if (!dev || !dev->data)
        return -ENODEV;

    mutex_lock(&dev->mutex);

    if (!dev->data_len ||
        (pos && *pos >= (loff_t)dev->data_len))
    {
        mutex_unlock(&dev->mutex);
        return 0;
    }

    avail = dev->data_len - (pos ? (size_t)*pos : 0);
    if (count > avail)
        count = avail;

    memcpy(dst, dev->data + (pos ? *pos : 0), count);

    if (pos)
        *pos += count;

    mutex_unlock(&dev->mutex);
    return count;
}
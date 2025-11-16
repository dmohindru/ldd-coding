#include "kmsgpipe.h"

int kmsgpipe_init(
    kmsgpipe_buffer_t *buf,
    uint8_t *base,
    kmsg_record_t *records,
    size_t capacity,
    size_t data_size)
{
    buf->base = base;
    buf->records = records;
    buf->head = 0;
    buf->tail = 0;
    buf->capacity = capacity;
    buf->data_size = data_size;

    /* Zero-initialize payload and metadata buffers */
    memset(base, 0, capacity * data_size);
    memset(records, 0, capacity * sizeof(kmsg_record_t));

    return 0;
}

ssize_t kmsgpipe_push(kmsgpipe_buffer_t *buf,
                      const uint8_t *data,
                      size_t len,
                      uid_t uid,
                      gid_t gid,
                      ktime_t timestamp)
{
    if (len > buf->data_size)
    {
        return -EMSGSIZE;
    }

    if (((buf->head + 1) % buf->capacity) == buf->tail)
    {
        return -ENOSPC;
    }

    uint8_t *src_addr = buf->base + (buf->head * buf->data_size);

    memcpy(src_addr, data, len);

    buf->records[buf->head].len = len;
    buf->records[buf->head].owner_uid = uid;
    buf->records[buf->head].owner_gid = gid;
    buf->records[buf->head].valid = true;
    buf->records[buf->head].timestamp = timestamp;

    buf->head = (buf->head + 1) % buf->capacity;

    return len;
}

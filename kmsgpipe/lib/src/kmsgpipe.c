#include "kmsgpipe.h"

static bool is_valid_access(
    uid_t requesting_uid,
    gid_t requesting_gid,
    uid_t record_uid,
    gid_t record_gid)
{
    if (requesting_uid == 0)
        return true;

    if (requesting_uid == record_uid)
        return true;

    if (requesting_gid == record_gid)
        return true;

    return false;
}

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
    for (size_t i = 0; i < capacity; i++)
        records[i].valid = false;

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
        return -EMSGSIZE;

    if (buf->records[buf->head].valid)
        return -ENOSPC;

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

ssize_t kmsgpipe_pop(
    kmsgpipe_buffer_t *buf,
    uint8_t *out_buf,
    uid_t uid,
    gid_t gid)
{
    if (!buf->records[buf->tail].valid)
        return -ENODATA;

    if (!is_valid_access(
            uid,
            gid,
            buf->records[buf->tail].owner_uid,
            buf->records[buf->tail].owner_gid))
        return -EACCES;

    uint8_t *src_addr = buf->base + (buf->tail * buf->data_size);
    memcpy(out_buf, src_addr, buf->records[buf->tail].len);
    buf->records[buf->tail].valid = false;
    ssize_t ret_val = buf->records[buf->tail].len;
    buf->tail = (buf->tail + 1) % buf->capacity;

    return ret_val;
}

ssize_t kmsgpipe_get_message_count(kmsgpipe_buffer_t *buf)
{
    ssize_t count = 0;

    /* Count all valid messages in the buffer */
    for (size_t i = 0; i < buf->capacity; i++)
    {
        if (buf->records[i].valid)
            count++;
    }

    return count;
}

ssize_t kmsgpipe_cleanup_expired(kmsgpipe_buffer_t *buf, uint64_t expiry_ms)
{
    int expired_count = 0;
    while (buf->records[buf->tail].valid && buf->records[buf->tail].timestamp < expiry_ms)
    {
        buf->records[buf->tail].valid = false;
        buf->tail = (buf->tail + 1) % buf->capacity;
        expired_count++;
    }
    return expired_count;
}

ssize_t kmsgpipe_clear(kmsgpipe_buffer_t *buf)
{
    ssize_t count = kmsgpipe_get_message_count(buf);
    /* Clear the data buffer: capacity * data_size (not data_size * data_size) */
    memset(buf->base, 0, buf->capacity * buf->data_size);
    memset(buf->records, 0, buf->capacity * sizeof(kmsg_record_t));
    buf->head = 0;
    buf->tail = 0;

    for (size_t i = 0; i < buf->capacity; i++)
        buf->records[i].valid = false;

    return count;
}

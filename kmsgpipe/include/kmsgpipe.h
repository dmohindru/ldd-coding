#ifndef KMSGPIPE_H
#define KMSGPIPE_H

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#else /* Userland */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
typedef uint64_t ktime_t;
#endif

typedef struct kmsg_record
{
    ktime_t timestamp;
    uid_t owner_uid;
    gid_t owner_gid;
    uint16_t len;
    bool valid;
} kmsg_record_t;

typedef struct kmsgpipe_buffer
{
    uint8_t *base;
    size_t capacity;
    size_t data_size;
    size_t head;
    size_t tail;
    kmsg_record_t *records;
} kmsgpipe_buffer_t;

/**
 * kmsgpipe_init - Initialize a message pipe buffer
 * @buf:       pointer to buffer struct to initialize
 * @base:      pointer to pre-allocated payload memory
 * @records:   pointer to pre-allocated metadata array
 * @capacity:  number of message slots
 * @data_size: bytes per message slot
 *
 * Returns:
 *   0 on success
 *  -EINVAL if arguments invalid
 */
int kmsgpipe_init(
    kmsgpipe_buffer_t *buf,
    uint8_t *base,
    kmsg_record_t *records,
    size_t capacity,
    size_t data_size);

/**
 * kmsgpipe_push - Push a data block into the circular buffer
 * @buf:        pointer to kmsgpipe_buffer
 * @data:       pointer to input data
 * @len:        length of data to push
 * @uid:        uid of caller
 * @gid:        gid of caller
 * @timestamp:  time of push operation
 *
 * Returns:
 *   >0  number of bytes copied
 *  -EINVAL invalid arguments
 *  -ENOSPC buffer full
 *  -EMSGSIZE message too large
 */
ssize_t kmsgpipe_push(
    kmsgpipe_buffer_t *buf,
    const uint8_t *data,
    size_t len,
    uid_t uid,
    gid_t gid,
    ktime_t timestamp);

/**
 * kmsgpipe_pop - Pop a data block from the circular buffer
 * @buf:        pointer to kmsgpipe_buffer
 * @out_buf:    output buffer (must be >= data_size bytes)
 * @uid:        uid of caller
 * @gid:        gid of caller
 *
 * Returns:
 *   >0  number of bytes copied
 *  -EINVAL invalid arguments
 *  -ENODATA buffer empty
 *  -EACCES unauthorized read
 */
ssize_t kmsgpipe_pop(
    kmsgpipe_buffer_t *buf,
    uint8_t *out_buf,
    uid_t uid,
    gid_t gid);

/**
 * kmsgpipe_cleanup_expired - Remove expired messages
 * @buf:        pointer to buffer
 * @expiry_ms:  absolute timestamp in milliseconds since Unix epoch
 *
 * Returns:
 *   >=0 number of messages deleted
 *   <0  error code
 */
ssize_t kmsgpipe_cleanup_expired(kmsgpipe_buffer_t *buf, uint64_t expiry_ms);

/**
 * kmsgpipe_get_message_count - Get number of valid messages
 * @buf: pointer to buffer
 *
 * Returns:
 *   >=0 message count
 *   <0  error code
 */
ssize_t kmsgpipe_get_message_count(kmsgpipe_buffer_t *buf);

#endif /* KMSGPIPE_H */
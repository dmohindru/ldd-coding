#ifndef KMSGPIPE_H
#define KMSGPIPE_H
#ifdef __KERNEL__
#include <linux/kernel.h>
#else /* Userland */
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
typedef int64_t ktime_t;
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
 *  -EINVAL if arguments invalid //TODO check if this is valid option
 */
int kmsgpipe_init(
    kmsgpipe_buffer_t *buf,
    uint8_t *base,
    kmsg_record_t *records,
    size_t capacity,
    size_t data_size);

/**
 * kmsgpipe_push - Push a data into circular buffer
 * @buf:        pointer to kmsgpipe_buffer
 * @data:       Actual data that needs to be pushed to circular buffer
 * @len:        Length of data being pushed
 * @uid_t:      uid of a calling process pushing the data
 * @gid_t:      gid of a calling process pushing the data
 * @timestamp:  timestamp when a push request is made
 ** Returns:
 *   0 on success
 *  -EINVAL if arguments invalid or buffer is full //TODO check if this is valid option
 */
int kmsgpipe_push(
    kmsgpipe_buffer_t *buf,
    const uint8_t *data,
    size_t len,
    uid_t uid,
    gid_t gid,
    ktime_t timestamp);

/**
 * kmsgpipe_pop - Pops a data from a circular buffer
 * @buf:        pointer to kmsgpipe_buffer
 * @out_buf:    Output buffer where the data would be copied
 * @uid_t:      uid of a calling process requesting the data
 * @gid_t:      gid of a calling process requesting the data
 * Returns:
 * - on success returns num of bytes transferred to @out_buf output buffer
 *  -EINVAL if arguments invalid or buffer is empty //TODO check if this is valid option maybe create an enum for full, empty etc
 */
/* TODO: Think about this requirement about privilege control.
If a message has been push by a privileged process it should only be able to retrieve by same privileged process.
 */
ssize_t kmsgpipe_pop(
    kmsgpipe_buffer_t *buf,
    uint8_t *out_buf,
    uid_t uid,
    gid_t gid);

/**
 * kmsgpipe_cleanup_expired - Cleanup/Remove the expired messages
 * @buf:        pointer to kmsgpipe_buffer
 * @expiry_ms:  age of messages in ms that need to be deleted
 * Returns:
 *  - on success returns num of messages cleaned/deleted
 *  -EINVAL if arguments invalid or operation failed //TODO check if this is valid option maybe create an enum for full, empty etc
 */
size_t kmsgpipe_cleanup_expired(kmsgpipe_buffer_t *buf, uint64_t expiry_ms);

/**
 * kmsgpipe_get_message_count - return number of messages in the buffer
 * @buf:        pointer to kmsgpipe_buffer
 * Returns:
 *  - on success returns num of messages available in buffer
 *  -EINVAL if arguments invalid or operation failed //TODO check if this is valid option maybe create an enum for full, empty etc
 */
size_t kmsgpipe_get_message_count(kmsgpipe_buffer_t *buf);

#endif

/*
Few important computations
uint8_t *slot_addr = base + (index * data_size);
kmsg_record_t *rec = &records[index];

write at index = head
head = (head + 1) % capacity

read at index = tail
tail = (tail + 1) % capacity
*/
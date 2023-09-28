#include "circular_buffer.h"

void circular_buffer_init(struct circular_buffer *p_buffer, char* data_buf, long size) {
    p_buffer->buffer = data_buf;
    p_buffer->size = size;
    p_buffer->head = p_buffer->tail = p_buffer->buffer;
    p_buffer->length = 0;
}

int is_full(struct circular_buffer *p_buffer) {
    return (p_buffer->head == p_buffer->tail) && (p_buffer->size == p_buffer->length);
}

long circular_buffer_capacity(struct circular_buffer *p_buffer) {
    return p_buffer->size - p_buffer->length;
}

long circular_buffer_write(struct circular_buffer *p_buffer, const char *data_buf, long size) {
    long i, buffer_cap = circular_buffer_capacity(p_buffer);

    if (size <= buffer_cap) {
        for (i = 0; i < size; i++) {
            *p_buffer->head = data_buf[i];
            if (++p_buffer->head == (p_buffer->buffer + p_buffer->size)) {
                p_buffer->head = p_buffer->buffer;
            }
        }
        p_buffer->length += size;
        return size;
    } else {
        return -1;
    }
}

long circular_buffer_read(struct circular_buffer *p_buffer, char *data_buf, long size) {
    long i;

    if (size <= p_buffer->length) {
        for (i = 0; i < size; i++) {
            data_buf[i] = *p_buffer->tail;
            *p_buffer->tail = 0;
            if (++p_buffer->tail == (p_buffer->buffer + p_buffer->size)) {
                p_buffer->tail = p_buffer->buffer;
            }
        }
        p_buffer->length -= size;
        return size;
    } else {
        return -1;
    }
}
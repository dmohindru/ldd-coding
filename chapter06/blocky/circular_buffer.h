#ifndef LINUX_DEVICE_DRIVER_DEVELOPMENT_CIRCULAR_BUFFER_H
#define LINUX_DEVICE_DRIVER_DEVELOPMENT_CIRCULAR_BUFFER_H

struct circular_buffer {
    char *buffer;
    char *head, *tail;
    long length, size;
};

void circular_buffer_init(struct circular_buffer *p_buffer, char* data_buf, long size);
int circular_buffer_is_full(struct circular_buffer *p_buffer);
long circular_buffer_capacity(struct circular_buffer *p_buffer);
long circular_buffer_write(struct circular_buffer *p_buffer, const char *data_buf, long size);
long circular_buffer_read(struct circular_buffer *p_buffer, char *data_buf, long size);

#endif //LINUX_DEVICE_DRIVER_DEVELOPMENT_CIRCULAR_BUFFER_H


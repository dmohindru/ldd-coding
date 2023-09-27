#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "circular_buffer.h"

void print_buffer_content(char *buf, long size) {
    long i;
    printf("[");
    for (i = 0; i < size; i++)
        printf(" %c", buf[i]);
    printf(" ]\n");

}

void clear_buffer(char *buf, long size) {
    memset(buf, 0, size);
}

int main(int argc, char **argv) {
    char char_data[] = {'a', 'b', 'c', 'd', 'e'};
    char read_data[5];
    long read_buffer_size = 5;
    long written_bytes, read_bytes;
    if (argc < 2) {
        printf("Usage: circular_buffer <buffer_size>\n");
        return 1;
    }
    int buffer_size = atoi(argv[1]);
    char *buff_ptr = (char *)malloc(buffer_size);
    memset(buff_ptr, 0, buffer_size);

    struct circular_buffer buffer;
    circular_buffer_init(&buffer, buff_ptr, buffer_size);

    // Write data to circular buffer and then read it
    if ((written_bytes = circular_buffer_write(&buffer, char_data, 3)) > 0) {
        printf("Bytes Written: %ld\n", written_bytes);
    }

    if ((read_bytes = circular_buffer_read(&buffer, read_data, 3)) > 0) {
        printf("Read bytes: %ld\n", read_bytes);
        print_buffer_content(read_data, 5);
        clear_buffer(read_data, read_buffer_size);
    }

    // Repeat the same process
    if ((written_bytes = circular_buffer_write(&buffer, char_data, 3)) > 0) {
        printf("Bytes Written: %ld\n", written_bytes);
    }

    if ((read_bytes = circular_buffer_read(&buffer, read_data, 3)) > 0) {
        printf("Read bytes: %ld\n", read_bytes);
        print_buffer_content(read_data, 5);
        clear_buffer(read_data, read_buffer_size);
    }

    // Writing more than the capacity of buffer
    if ((written_bytes = circular_buffer_write(&buffer, char_data, 6)) > 0) {
        printf("Bytes Written: %ld\n", written_bytes);
    } else {
        printf("Bytes not written\n");
    }


    // Read more data than available in buffer
    if ((read_bytes = circular_buffer_read(&buffer, read_data, 3)) > 0) {
        printf("Read bytes: %ld\n", read_bytes);
        print_buffer_content(read_data, 5);
        clear_buffer(read_data, read_buffer_size);
    } else {
        printf("Bytes not available");
    }


    return 0;
    
}

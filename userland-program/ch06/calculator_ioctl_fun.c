#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv) {
    int fd = open("/dev/calculator", O_RDWR);
    long first_op = 500, result;
    
    if (fd == -1) {
        perror("open");
        return 1;
    }

    ssize_t num_bytes = read(fd, &result, sizeof(long));
    printf("Current value of result [%ld], number of bytes read [%ld]\n", result, num_bytes);

    printf("Writing first operand [%ld] to calculator device\n", first_op);
    ssize_t bytes_written = write(fd, &first_op, sizeof(long));

    num_bytes = read(fd, &result, sizeof(long));
    printf("Updated value of result [%ld], number of bytes read [%ld]\n", result, num_bytes);

    return 0;
}
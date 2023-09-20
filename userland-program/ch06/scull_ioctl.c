/*
 * This user land program demonstrates how to use ioctl system calls to manipulate device in this case scull
 * Written by: Dhruv Mohindru
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "scull_ioctl.h"



int main() {
    int fd = open("/dev/scull0", O_RDWR);
    int quantam_size, qset_size;

    if (fd == -1) {
        perror("open");
        return 1;
    }

    // Reset the device
    printf("Resetting scull device\n");
    if (ioctl(fd, SCULL_IOCRESET, 0) < 0) {
        perror("ioctl");
        return 1;
    }

    // Get the current quantam and qset values
    if (ioctl(fd, SCULL_IOCGQUANTUM, &quantam_size) < 0) {
        perror("ioctl");
        return 1;
    }
    printf("original quantam size: %d\n", quantam_size);

    if (ioctl(fd, SCULL_IOCGQSET, &qset_size) < 0) {
        perror("ioctl");
        return 1;
    }
    printf("original qset size: %d\n", qset_size);

    // Set the value for quantam and qset on a device
    // Note these call will only succeed if this program is run a system admin (sudo)
    quantam_size = 2000, qset_size = 500;
    printf("Setting quantam = %d and qset = %d\n", quantam_size, qset_size );

    if (ioctl(fd, SCULL_IOCSQUANTUM, &quantam_size) < 0) {
        perror("ioctl");
        return 1;
    }

    if (ioctl(fd, SCULL_IOCSQSET, &qset_size) < 0) {
        perror("ioctl");
        return 1;
    }

    // Get the updated value for quantam and qset values
    printf("New quantam and qset values\n");

    if (ioctl(fd, SCULL_IOCGQUANTUM, &quantam_size) < 0) {
        perror("ioctl");
        return 1;
    }
    printf("new quantam size: %d\n", quantam_size);

    if (ioctl(fd, SCULL_IOCGQSET, &qset_size) < 0) {
        perror("ioctl");
        return 1;
    }
    printf("new qset size: %d\n", qset_size);


    close(fd);

    return 0;
}
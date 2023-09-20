#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "calculator_ioctl.h"

enum operator {
    ADD, SUB, DIV, MUL
};

char *operator_string[] ={"Add", "Sub", "Div", "Mul"};

int main(int argc, char **argv) {
    int fd = open("/dev/calculator", O_RDWR);
    long first_op = 500, result, second_op;
    enum operator op;
    ssize_t num_bytes, bytes_written;
    
    if (fd == -1) {
        perror("open");
        return 1;
    }

    // query for second operator
    if (ioctl(fd, CALCULATOR_QUERY_OPERAND, &second_op) < 0) {
        perror("ioctl");
        return 1;
    }

    // query current operator for driver
    if (ioctl(fd, CALCULATOR_QUERY_OPERATOR, &op) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("Current operator [%s], second operand [%ld]\n", operator_string[op], second_op);

    printf("Writing first operand [%ld] to calculator device\n", first_op);
    write(fd, &first_op, sizeof(long));

    read(fd, &result, sizeof(long));
    printf("Updated value of result [%ld]\n", result);

    second_op = 100;
    if (ioctl(fd, CALCULATOR_SET_OPERAND, &second_op) < 0) {
        perror("ioctl");
        return 1;
    }

    printf("Writing first operand [%ld] to calculator device\n", first_op);
    write(fd, &first_op, sizeof(long));

    read(fd, &result, sizeof(long));
    printf("Updated value of result [%ld]\n", result);

    op = SUB;
    if (ioctl(fd, CALCULATOR_SET_OPERATOR, &op) < 0) {
        perror("ioctl");
        return 1;
    }


    // query for current operator

//    ssize_t num_bytes = read(fd, &result, sizeof(long));
//    printf("Current value of result [%ld], number of bytes read [%ld]\n", result, num_bytes);
//
//    printf("Writing first operand [%ld] to calculator device\n", first_op);
//    ssize_t bytes_written = write(fd, &first_op, sizeof(long));
//
//    num_bytes = read(fd, &result, sizeof(long));
//    printf("Updated value of result [%ld], number of bytes read [%ld]\n", result, num_bytes);

    return 0;
}
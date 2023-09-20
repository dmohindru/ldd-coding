#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "calculator_ioctl.h"

enum operator {
    ADD, SUB, DIV, MUL
};

char *operator_string[] ={"Add", "Sub", "Div", "Mul"};

int reset_device(int fd) {
    if (ioctl(fd, CALCULATOR_RESET) < 0) {
        perror("ioctl");
        return 1;
    }
    return 0;
}

int print_current_state(int fd) {
    long second_op;
    enum operator op;
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

    return 0;
}

int set_second_operand(int fd, long second_operand) {
    if (ioctl(fd, CALCULATOR_SET_OPERAND, &second_operand) < 0) {
        perror("ioctl");
        return 1;
    }
    return 0;
}

int set_operator(int fd, enum operator op) {
    if (ioctl(fd, CALCULATOR_SET_OPERATOR, &op) < 0) {
        perror("ioctl");
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    int fd = open("/dev/calculator", O_RDWR);
    long first_op = 500, result, second_op;
    enum operator op;

    
    if (fd == -1) {
        perror("open");
        return 1;
    }

    reset_device(fd);

    print_current_state(fd);

    second_op = 100;
    set_second_operand(fd, second_op);

    print_current_state(fd);

    printf("Writing first operand [%ld] to calculator device\n", first_op);
    write(fd, &first_op, sizeof(long));

    read(fd, &result, sizeof(long));
    printf("Updated value of result [%ld]\n", result);

    reset_device(fd);

    // this operation can only be done by admin users
    set_operator(fd, SUB);
    second_op = 200;
    set_second_operand(fd, second_op);

    print_current_state(fd);

    printf("Writing first operand [%ld] to calculator device\n", first_op);
    write(fd, &first_op, sizeof(long));

    read(fd, &result, sizeof(long));
    printf("Updated value of result [%ld]\n", result);

    return 0;
}
//
// Created by dhruv on 16/09/23.
//

#ifndef LDD_CALCULATOR_H
#define LDD_CALCULATOR_H

#include <linux/cdev.h>

enum operator {
    ADD, SUB, DIV, MUL
};

struct calculator_driver {
    void *data;
    long first_op;
    long second_op;
    long result;
    enum operator op;
    struct mutex mutex;
    struct cdev cdev;
};

// ioctl number
#define CALCULATOR_IOC_MAGIC 'Z'

#define CALCULATOR_SET_OPERATOR _IOW(CALCULATOR_IOC_MAGIC, 1, int)
#define CALCULATOR_SET_OPERAND _IOW(CALCULATOR_IOC_MAGIC, 2, long)
#define CALCULATOR_RESET _IO(CALCULATOR_IOC_MAGIC, 3)
#define CALCULATOR_QUERY_OPERATOR _IOR(CALCULATOR_IOC_MAGIC, 4, int)
#define CALCULATOR_QUERY_OPERAND _IOR(CALCULATOR_IOC_MAGIC, 5, long)

#define CALCULATOR_IOC_MAX 6

#endif //LDD_CALCULATOR_H

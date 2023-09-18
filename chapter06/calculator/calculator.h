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

#endif //LDD_CALCULATOR_H

#pragma once

#include "common.hpp"

#define TUPLE_SIZE  4

typedef struct __attribute__((packed, aligned(4 * TUPLE_SIZE))) {
    float values[TUPLE_SIZE];
} tuple_t;

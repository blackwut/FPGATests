#pragma once

#include "common.hpp"

typedef struct __attribute__((packed, aligned(4 * TUPLE_SIZE))) {
    float values[TUPLE_SIZE];
} tuple_t;

typedef struct __attribute__((packed, aligned(4 * TUPLE_SIZE))) {
    tuple_t ts[COMPUTE_UNITS];
} big_tuple_t;
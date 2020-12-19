#pragma once

#define TUPLE_SIZE              4
#define COMPUTE_UNITS           8


enum K_TYPE {
    K_UNROLL,
    K_REPLICA,
    K_NDRANGE
};


#define K_UNROLL_NUMS           3
const char * K_UNROLL_NAMES[] {
    "source",
    "map",
    "sink"
};
const char * P_UNROLL_FILENAME = "./unroll.aocx";


#define K_REPLICA_NUMS          10
const char * K_REPLICA_NAMES[] {
    "source",
    "map0",
    "map1",
    "map2",
    "map3",
    "map4",
    "map5",
    "map6",
    "map7",
    "sink"
};
const char * P_REPLICA_FILENAME = "./replica.aocx";


#define K_NDRANGE_NUMS          3
const char * K_NDRANGE_NAMES[] {
    "source",
    "map",
    "sink"
};
const char * P_NDRANGE_FILENAME = "./ndrange.aocx";

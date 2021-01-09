#pragma once

#define TUPLE_SIZE              4
#define COMPUTE_UNITS           8

enum K_TYPE {
    K_BASE,
    K_UNROLL,
    K_REPLICA,
    K_REPLICA_NEW,
    K_NDRANGE
};

#define K_BASE_NUMS           3
const char * K_BASE_NAMES[] {
    "source",
    "map",
    "sink"
};
const char * P_BASE_FILENAME = "./base.aocx";


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

#define K_REPLICA_NEW_NUMS          24
const char * K_REPLICA_NEW_NAMES[] {
    "source0",
    "source1",
    "source2",
    "source3",
    "source4",
    "source5",
    "source6",
    "source7",
    "map0",
    "map1",
    "map2",
    "map3",
    "map4",
    "map5",
    "map6",
    "map7",
    "sink0",
    "sink1",
    "sink2",
    "sink3",
    "sink4",
    "sink5",
    "sink6",
    "sink7"
};
const char * P_REPLICA_NEW_FILENAME = "./replica_new.aocx";


#define K_NDRANGE_NUMS          3
const char * K_NDRANGE_NAMES[] {
    "source",
    "map",
    "sink"
};
const char * P_NDRANGE_FILENAME = "./ndrange.aocx";


#define K_FUSE_NUMS             3
const char * K_FUSE_NAMES[] {
    "source",
    "map",
    "sink"
};
const char * P_FUSE_FILENAME = "./fuse.aocx";


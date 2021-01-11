#pragma OPENCL EXTENSION cl_intel_channels : enable
#define CHANNEL_DEPTH   16
#define TUPLE_SIZE      4
#define COMPUTE_UNITS   4

typedef struct __attribute__((packed, aligned(4 * TUPLE_SIZE))) {
    float values[TUPLE_SIZE];
} tuple_t;

typedef struct __attribute__((packed, aligned(4 * TUPLE_SIZE))) {
    tuple_t ts[COMPUTE_UNITS];
} big_tuple_t;

#define tuple_in_t  tuple_t
#define tuple_map_t tuple_t
#define tuple_out_t tuple_t

channel big_tuple_t source_map __attribute__((depth(CHANNEL_DEPTH)));
channel big_tuple_t map_sink   __attribute__((depth(CHANNEL_DEPTH)));

__attribute__((uses_global_work_offset(0)))
__attribute__((max_global_work_dim(0)))
__kernel
void source(__global const tuple_in_t * restrict data, const uint N)
{
    for (uint n = 0; n < N; ++n) {
        big_tuple_t bt;

        #pragma unroll
        for (uint c = 0; c < COMPUTE_UNITS; ++c) {
            bt.ts[c] = data[n * COMPUTE_UNITS + c];
        }

        write_channel_intel(source_map, bt);
    }
}

__attribute__((uses_global_work_offset(0)))
__attribute__((max_global_work_dim(0)))
__kernel
void map(const uint N)
{
    for (uint n = 0; n < N; ++n) {
        big_tuple_t _in = read_channel_intel(source_map);

        big_tuple_t _out;
        #pragma unroll
        for (uint c = 0; c < COMPUTE_UNITS; ++c) {

            #pragma unroll
            for (uint i = 0; i < TUPLE_SIZE; ++i) {
                 _out.ts[c].values[i] = sin(_in.ts[c].values[i]);
             }
        }

        write_channel_intel(map_sink, _out);
    }
}

__attribute__((uses_global_work_offset(0)))
__attribute__((max_global_work_dim(0)))
__kernel
void sink(__global tuple_out_t * restrict data, const uint N)
{
    for (uint n = 0; n < N; ++n) {
        big_tuple_t bt = read_channel_intel(map_sink);

        #pragma unroll
        for (uint c = 0; c < COMPUTE_UNITS; ++c) {
            data[n * COMPUTE_UNITS + c] = bt.ts[c];
        }
    }
}

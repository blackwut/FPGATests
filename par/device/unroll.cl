#pragma OPENCL EXTENSION cl_intel_channels : enable
#define CHANNEL_DEPTH   16
#define TUPLE_SIZE      4
#define COMPUTE_UNITS   8

typedef struct __attribute__((packed, aligned(4 * TUPLE_SIZE))) {
    float values[TUPLE_SIZE];
} tuple_t;

#define tuple_in_t  tuple_t
#define tuple_map_t tuple_t
#define tuple_out_t tuple_t

channel tuple_map_t source_map[COMPUTE_UNITS] __attribute__((depth(CHANNEL_DEPTH)));
channel tuple_out_t map_sink[COMPUTE_UNITS]   __attribute__((depth(CHANNEL_DEPTH)));

__attribute__((uses_global_work_offset(0)))
__attribute__((max_global_work_dim(0)))
__kernel
void source(__global const tuple_in_t * restrict data, const uint N)
{
    for (uint n = 0; n < N; ++n) {
        #pragma unroll
        for (uint c = 0; c < COMPUTE_UNITS; ++c) {
            write_channel_intel(source_map[c], data[n * COMPUTE_UNITS + c]);
        }
    }
}

__attribute__((uses_global_work_offset(0)))
__attribute__((max_global_work_dim(0)))
__kernel
void map(const uint N)
{
    for (uint n = 0; n < N; ++n) {

        // input
        tuple_map_t _in[COMPUTE_UNITS];
        #pragma unroll
        for (uint c = 0; c < COMPUTE_UNITS; ++c) {
            _in[c] = read_channel_intel(source_map[c]);
        }

        // compute
        tuple_map_t _tmp[COMPUTE_UNITS];
        tuple_out_t _out[COMPUTE_UNITS];
        #pragma unroll
        for (uint c = 0; c < COMPUTE_UNITS; ++c) {
            _tmp[c] = _in[c];
            for (uint i = 0; i < TUPLE_SIZE; ++i) {
                 _out[c].values[i] = sin(_tmp[c].values[i]);
            }
        }

        // output
        #pragma unroll
        for (uint c = 0; c < COMPUTE_UNITS; ++c) {
            write_channel_intel(map_sink[c], _out[c]);
        }
    }
}

__attribute__((uses_global_work_offset(0)))
__attribute__((max_global_work_dim(0)))
__kernel
void sink(__global tuple_out_t * restrict data, const uint N)
{
    for (uint n = 0; n < N; ++n) {
        #pragma unroll
        for (uint c = 0; c < COMPUTE_UNITS; ++c) {
            data[n * COMPUTE_UNITS + c] = read_channel_intel(map_sink[c]);
        }
    }
}

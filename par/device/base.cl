#pragma OPENCL EXTENSION cl_intel_channels : enable
#define CHANNEL_DEPTH   16
#define TUPLE_SIZE      4

typedef struct __attribute__((packed, aligned(4 * TUPLE_SIZE))) {
    float values[TUPLE_SIZE];
} tuple_t;

#define tuple_in_t  tuple_t
#define tuple_map_t tuple_t
#define tuple_out_t tuple_t

channel tuple_map_t source_map __attribute__((depth(CHANNEL_DEPTH)));
channel tuple_out_t map_sink   __attribute__((depth(CHANNEL_DEPTH)));

__attribute__((uses_global_work_offset(0)))
__attribute__((max_global_work_dim(0)))
__kernel
void source(__global const tuple_in_t * restrict data, const uint N)
{
    for (uint n = 0; n < N; ++n) {
        write_channel_intel(source_map, data[n]);
    }
}

__attribute__((uses_global_work_offset(0)))
__attribute__((max_global_work_dim(0)))
__kernel
void map(const uint N)
{
    for (uint n = 0; n < N; ++n) {
        tuple_map_t _in = read_channel_intel(source_map);
        tuple_out_t _out;
        #pragma unroll
        for (uint i = 0; i < TUPLE_SIZE; ++i) {
            _out.values[i] = sin(_in.values[i]);
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
        data[n] = read_channel_intel(map_sink);
    }
}

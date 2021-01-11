#pragma OPENCL EXTENSION cl_intel_channels : enable
#define CHANNEL_DEPTH   16
#define TUPLE_SIZE      4
#define COMPUTE_UNITS   8


// #define KERNEL_MAP(IDX)                                         \
// __kernel                                                        \
// void map ## IDX (const uint N) {                                \
//     for (uint n = 0; n < N; ++n) {                              \
//         tuple_map_t _in = read_channel_intel(source_map[IDX]);  \
//         tuple_map_t _tmp = _in;                                 \
//         tuple_out_t _out;                                       \
//         for (uint i = 0; i < TUPLE_SIZE; ++i) {                 \
//             _out.values[i] = sin(_tmp.values[i]);               \
//         }                                                       \
//         write_channel_intel(map_sink[IDX], _out);               \
//     }                                                           \
// }
#define KERNEL_MAP(IDX)                                         \
__kernel                                                        \
void map ## IDX (const uint N) {                                \
    for (uint n = 0; n < N; ++n) {                              \
        tuple_map_t _in = read_channel_intel(source_map[IDX]);  \
        tuple_out_t _out;                                       \
        for (uint i = 0; i < TUPLE_SIZE; ++i) {                 \
            _out.values[i] = sin(_in.values[i]);                \
        }                                                       \
        write_channel_intel(map_sink[IDX], _out);               \
    }                                                           \
}


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


// COMPUTE KENRELS
#if (0 < COMPUTE_UNITS)
    KERNEL_MAP(0)
#endif
#if (1 < COMPUTE_UNITS)
    KERNEL_MAP(1)
#endif
#if (2 < COMPUTE_UNITS)
    KERNEL_MAP(2)
#endif
#if (3 < COMPUTE_UNITS)
    KERNEL_MAP(3)
#endif
#if (4 < COMPUTE_UNITS)
    KERNEL_MAP(4)
#endif
#if (5 < COMPUTE_UNITS)
    KERNEL_MAP(5)
#endif
#if (6 < COMPUTE_UNITS)
    KERNEL_MAP(6)
#endif
#if (7 < COMPUTE_UNITS)
    KERNEL_MAP(7)
#endif


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

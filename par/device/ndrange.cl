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
__attribute__((reqd_work_group_size(COMPUTE_UNITS, 1, 1)))
__attribute__((max_work_group_size(COMPUTE_UNITS, 1, 1)))
__kernel
void source(__global const tuple_in_t * restrict data, const uint N)
{

    const uint gid = get_global_id(0);

    for (uint n = 0; n < N; ++n) {
        tuple_in_t _in = data[n * COMPUTE_UNITS + gid];
        tuple_map_t _out = _in;
        switch (gid) {
            case 0: write_channel_intel(source_map[0], _out); break;
            case 1: write_channel_intel(source_map[1], _out); break;
            case 2: write_channel_intel(source_map[2], _out); break;
            case 3: write_channel_intel(source_map[3], _out); break;
            case 4: write_channel_intel(source_map[4], _out); break;
            case 5: write_channel_intel(source_map[5], _out); break;
            case 6: write_channel_intel(source_map[6], _out); break;
            case 7: write_channel_intel(source_map[7], _out); break;
        }
    }
}

__attribute__((uses_global_work_offset(0)))
__attribute__((reqd_work_group_size(COMPUTE_UNITS, 1, 1)))
__attribute__((max_work_group_size(COMPUTE_UNITS, 1, 1)))
__kernel
void map(const uint N)
{
    const uint gid = get_global_id(0);

    for (uint n = 0; n < N; ++n) {
        tuple_map_t _in;
        switch (gid) {
            case 0: _in = read_channel_intel(source_map[0]); break;
            case 1: _in = read_channel_intel(source_map[1]); break;
            case 2: _in = read_channel_intel(source_map[2]); break;
            case 3: _in = read_channel_intel(source_map[3]); break;
            case 4: _in = read_channel_intel(source_map[4]); break;
            case 5: _in = read_channel_intel(source_map[5]); break;
            case 6: _in = read_channel_intel(source_map[6]); break;
            case 7: _in = read_channel_intel(source_map[7]); break;
        }

        tuple_map_t _tmp = _in;
        tuple_out_t _out;
        for (uint i = 0; i < TUPLE_SIZE; ++i) {
            _out.values[i] = sin(_tmp.values[i]);
        }

        switch (gid) {
            case 0: write_channel_intel(map_sink[0], _out); break;
            case 1: write_channel_intel(map_sink[1], _out); break;
            case 2: write_channel_intel(map_sink[2], _out); break;
            case 3: write_channel_intel(map_sink[3], _out); break;
            case 4: write_channel_intel(map_sink[4], _out); break;
            case 5: write_channel_intel(map_sink[5], _out); break;
            case 6: write_channel_intel(map_sink[6], _out); break;
            case 7: write_channel_intel(map_sink[7], _out); break;
        }
    }
}

__attribute__((uses_global_work_offset(0)))
__attribute__((reqd_work_group_size(COMPUTE_UNITS, 1, 1)))
__attribute__((max_work_group_size(COMPUTE_UNITS, 1, 1)))
__kernel
void sink(__global tuple_out_t * restrict data, const uint N)
{
    const uint gid = get_global_id(0);

    for (uint n = 0; n < N; ++n) {
        tuple_t _in;
        switch (gid) {
            case 0: _in = read_channel_intel(map_sink[0]); break;
            case 1: _in = read_channel_intel(map_sink[1]); break;
            case 2: _in = read_channel_intel(map_sink[2]); break;
            case 3: _in = read_channel_intel(map_sink[3]); break;
            case 4: _in = read_channel_intel(map_sink[4]); break;
            case 5: _in = read_channel_intel(map_sink[5]); break;
            case 6: _in = read_channel_intel(map_sink[6]); break;
            case 7: _in = read_channel_intel(map_sink[7]); break;
        }
        data[n * COMPUTE_UNITS + gid] = _in;
    }
}

#pragma OPENCL EXTENSION cl_intel_channels : enable

#define PRIMITIVE_CAT(a, b) a ## b
#define CAT(a, b) PRIMITIVE_CAT(a, b)

#define CL_SINGLE_TASK                      \
__attribute__((uses_global_work_offset(0))) \
__attribute__((max_global_work_dim(0)))     \
__kernel void


#define PAR_W   4
channel int s_m[PAR_W];
channel int m_s[PAR_W];

//------------------------------------------------------------------------------
//
//  GENERATOR
//
//------------------------------------------------------------------------------
CL_SINGLE_TASK emitter(const uint N)
{
    uint w = 0;
    for (uint n = 0; n < N; ++n) {
        switch (w) {
            case 0: write_channel_intel(s_m[0], n); break;
            case 1: write_channel_intel(s_m[1], n); break;
            case 2: write_channel_intel(s_m[2], n); break;
            case 3: write_channel_intel(s_m[3], n); break;
        }

        if (w == PAR_W - 1) {
            w = 0;
        } else {
            w++;
        }
    }
}

//------------------------------------------------------------------------------
//
//  WORKER
//
//------------------------------------------------------------------------------
#undef  IDX
#define IDX 0
CL_SINGLE_TASK CAT(worker, IDX)(const uint N)
{
    for (uint n = 0; n < N; ++n) {
        int v = read_channel_intel(s_m[IDX]);
        v += 1;
        write_channel_intel(m_s[IDX], v);
    }
}

#undef  IDX
#define IDX 1
CL_SINGLE_TASK CAT(worker, IDX)(const uint N)
{
    for (uint n = 0; n < N; ++n) {
        int v = read_channel_intel(s_m[IDX]);
        v += 1;
        write_channel_intel(m_s[IDX], v);
    }
}

#undef  IDX
#define IDX 2
CL_SINGLE_TASK CAT(worker, IDX)(const uint N)
{
    for (uint n = 0; n < N; ++n) {
        int v = read_channel_intel(s_m[IDX]);
        v += 1;
        write_channel_intel(m_s[IDX], v);
    }
}

#undef  IDX
#define IDX 3
CL_SINGLE_TASK CAT(worker, IDX)(const uint N)
{
    for (uint n = 0; n < N; ++n) {
        int v = read_channel_intel(s_m[IDX]);
        v += 1;
        write_channel_intel(m_s[IDX], v);
    }
}

//------------------------------------------------------------------------------
//
//  COLLECTOR
//
//------------------------------------------------------------------------------
CL_SINGLE_TASK collector(const uint N)
{
    uint w = 0;
    for (uint n = 0; n < N; ++n) {
        int v;
        switch (w) {
            case 0: v = read_channel_intel(m_s[0]); break;
            case 1: v = read_channel_intel(m_s[1]); break;
            case 2: v = read_channel_intel(m_s[2]); break;
            case 3: v = read_channel_intel(m_s[3]); break;
        }

        if (w == PAR_W - 1) {
            w = 0;
        } else {
            w++;
        }
    }
}

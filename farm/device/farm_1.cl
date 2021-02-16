#pragma OPENCL EXTENSION cl_intel_channels : enable

#define PRIMITIVE_CAT(a, b) a ## b
#define CAT(a, b) PRIMITIVE_CAT(a, b)

#define CL_SINGLE_TASK                      \
__attribute__((uses_global_work_offset(0))) \
__attribute__((max_global_work_dim(0)))     \
__kernel void


channel int s_m;
channel int m_s;

//------------------------------------------------------------------------------
//
//  GENERATOR
//
//------------------------------------------------------------------------------
CL_SINGLE_TASK emitter(const uint N)
{
    for (uint n = 0; n < N; ++n) {
        write_channel_intel(s_m, n);
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
        int v = read_channel_intel(s_m);
        v += 1;
        write_channel_intel(m_s, v);
    }
}

//------------------------------------------------------------------------------
//
//  COLLECTOR
//
//------------------------------------------------------------------------------
CL_SINGLE_TASK collector(const uint N)
{
    for (uint n = 0; n < N; ++n) {
        int v = read_channel_intel(m_s);
    }
}

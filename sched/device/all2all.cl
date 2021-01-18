#pragma OPENCL EXTENSION cl_intel_channels : enable
// #pragma OPENCL EXTENSION cl_intel_arbitrary_precision_integers : enable
// typedef unsigned int __attribute__((__ap_int(1))) uint1_t;

#define CHANNEL_DEPTH   1
#define PAR_G           2
#define PAR_M           3
#define PAR_S           2


// TODO: all the following have to be defined in "common.h"
#define PRIMITIVE_CAT(a, b) a ## b
#define CAT(a, b) PRIMITIVE_CAT(a, b)

#define CL_AUTORUN                      \
__attribute__((max_global_work_dim(0))) \
__attribute__((autorun))                \
__kernel void

#define CL_SINGLE_TASK                      \
__attribute__((uses_global_work_offset(0))) \
__attribute__((max_global_work_dim(0)))     \
__kernel void


typedef struct {
    uint index; //TODO: store to global memory also the index of each tuple
    bool EOS;

    float data;
} tuple_t;

channel tuple_t g_m[PAR_G][PAR_M] __attribute__((depth(CHANNEL_DEPTH)));
channel tuple_t m_s[PAR_M][PAR_S] __attribute__((depth(CHANNEL_DEPTH)));

//------------------------------------------------------------------------------
//
//  GENERATOR
//
//------------------------------------------------------------------------------
#undef IDX
#define IDX 0
CL_SINGLE_TASK CAT(generator, IDX)(const uint N)
{
    for (uint n = 0; n < N; ++n) {
        tuple_t t;
        t.index = n;
        t.EOS = false;
        t.data = 0.42f * n;

        bool success = false;
        do {
                          success = write_channel_nb_intel(g_m[IDX][(0 + IDX) % PAR_M], t);
            if (!success) success = write_channel_nb_intel(g_m[IDX][(1 + IDX) % PAR_M], t);
            if (!success) success = write_channel_nb_intel(g_m[IDX][(2 + IDX) % PAR_M], t);
        } while (!success);
    }

    tuple_t t;
    t.EOS = true;
    #pragma unroll
    for (uint i = 0; i < PAR_M; ++i) {
        write_channel_intel(g_m[IDX][i], t);
    }
}

#undef IDX
#define IDX 1
CL_SINGLE_TASK CAT(generator, IDX)(const uint N)
{
    for (uint n = 0; n < N; ++n) {
        tuple_t t;
        t.index = n;
        t.EOS = false;
        t.data = 0.42f * n;

        bool success = false;
        do {
                          success = write_channel_nb_intel(g_m[IDX][(0 + IDX) % PAR_M], t);
            if (!success) success = write_channel_nb_intel(g_m[IDX][(1 + IDX) % PAR_M], t);
            if (!success) success = write_channel_nb_intel(g_m[IDX][(2 + IDX) % PAR_M], t);
        } while (!success);
    }

    tuple_t t;
    t.EOS = true;
    #pragma unroll
    for (uint i = 0; i < PAR_M; ++i) {
        write_channel_intel(g_m[IDX][i], t);
    }
}


//------------------------------------------------------------------------------
//
//  MAP
//
//------------------------------------------------------------------------------

// TODO: fix low Fmax (194.4)

inline float process(float x) {
    return sin(x * x) * cos((x + 10) / x);
}

#undef IDX
#define IDX 0
CL_AUTORUN CAT(map, IDX)()
{
    while (1) {
        uint c = IDX % PAR_S;

        uint EOS = 0;
        bool done = false;

        while (!done) {
            bool valid = false;
            tuple_t t;
            switch (c) {
                case 0:
                    t = read_channel_nb_intel(g_m[0][IDX], &valid);
                    break;
                case 1:
                    t = read_channel_nb_intel(g_m[1][IDX], &valid);
                    break;
            }

            if (valid) {
                if (!t.EOS) {
                    t.data = process(t.data);

                    bool success = false;
                    do {
                                      success = write_channel_nb_intel(m_s[IDX][(0 + IDX) % PAR_S], t);
                        if (!success) success = write_channel_nb_intel(m_s[IDX][(1 + IDX) % PAR_S], t);
                    } while (!success);
                } else {
                    EOS++;
                }
            }

            if (EOS == PAR_S) {
                done = true;
            }

            c++;
            if (c == PAR_G) {
                c = 0;
            }
        }

        tuple_t t;
        t.EOS = true;
        #pragma unroll
        for (uint i = 0; i < PAR_S; ++i) {
            write_channel_intel(m_s[IDX][i], t);
        }
    }
}

#undef IDX
#define IDX 1
CL_AUTORUN CAT(map, IDX)()
{
    while (1) {
        uint c = IDX % PAR_S;

        uint EOS = 0;
        bool done = false;

        while (!done) {
            bool valid = false;
            tuple_t t;
            switch (c) {
                case 0:
                    t = read_channel_nb_intel(g_m[0][IDX], &valid);
                    break;
                case 1:
                    t = read_channel_nb_intel(g_m[1][IDX], &valid);
                    break;
            }

            if (valid) {
                if (!t.EOS) {
                    t.data = process(t.data);

                    bool success = false;
                    do {
                                      success = write_channel_nb_intel(m_s[IDX][(0 + IDX) % PAR_S], t);
                        if (!success) success = write_channel_nb_intel(m_s[IDX][(1 + IDX) % PAR_S], t);
                    } while (!success);
                } else {
                    EOS++;
                }
            }

            if (EOS == PAR_S) {
                done = true;
            }

            c++;
            if (c == PAR_G) {
                c = 0;
            }
        }

        tuple_t t;
        t.EOS = true;
        #pragma unroll
        for (uint i = 0; i < PAR_S; ++i) {
            write_channel_intel(m_s[IDX][i], t);
        }
    }
}

#undef IDX
#define IDX 2
CL_AUTORUN CAT(map, IDX)()
{
    while (1) {
        uint c = IDX % PAR_S;

        uint EOS = 0;
        bool done = false;

        while (!done) {
            bool valid = false;
            tuple_t t;
            switch (c) {
                case 0:
                    t = read_channel_nb_intel(g_m[0][IDX], &valid);
                    break;
                case 1:
                    t = read_channel_nb_intel(g_m[1][IDX], &valid);
                    break;
            }

            if (valid) {
                if (!t.EOS) {
                    t.data = process(t.data);

                    bool success = false;
                    do {
                                      success = write_channel_nb_intel(m_s[IDX][(0 + IDX) % PAR_S], t);
                        if (!success) success = write_channel_nb_intel(m_s[IDX][(1 + IDX) % PAR_S], t);
                    } while (!success);
                } else {
                    EOS++;
                }
            }

            if (EOS == PAR_S) {
                done = true;
            }

            c++;
            if (c == PAR_G) {
                c = 0;
            }
        }

        tuple_t t;
        t.EOS = true;
        #pragma unroll
        for (uint i = 0; i < PAR_S; ++i) {
            write_channel_intel(m_s[IDX][i], t);
        }
    }
}


//------------------------------------------------------------------------------
//
//  SINK
//
//------------------------------------------------------------------------------

#undef IDX
#define IDX 0
CL_SINGLE_TASK CAT(sink, IDX)(__global float * restrict data, __global uint * restrict N)
{
    uint c = IDX % PAR_M;
    uint n = 0;

    uint EOS = 0;
    bool done = false;

    while (!done) {
        bool valid = false;
        tuple_t t;
        switch (c) {
            case 0:
                t = read_channel_nb_intel(m_s[0][IDX], &valid);
                break;
            case 1:
                t = read_channel_nb_intel(m_s[1][IDX], &valid);
                break;
            case 2:
                t = read_channel_nb_intel(m_s[2][IDX], &valid);
                break;
        }

        if (valid) {
            if (!t.EOS) {
                data[n] = t.data;
                n++;
            } else {
                EOS++;
            }
        }

        if (EOS == PAR_M) {
            done = true;
        }

        c++;
        if (c == PAR_M) {
            c = 0;
        }
    }

    N[0] = n;
}

#undef IDX
#define IDX 1
CL_SINGLE_TASK CAT(sink, IDX)(__global float * restrict data, __global uint * restrict N)
{
    uint c = IDX % PAR_M;
    uint n = 0;

    uint EOS = 0;
    bool done = false;

    while (!done) {
        bool valid = false;
        tuple_t t;
        switch (c) {
            case 0:
                t = read_channel_nb_intel(m_s[0][IDX], &valid);
                break;
            case 1:
                t = read_channel_nb_intel(m_s[1][IDX], &valid);
                break;
            case 2:
                t = read_channel_nb_intel(m_s[2][IDX], &valid);
                break;
        }

        if (valid) {
            if (!t.EOS) {
                data[n] = t.data;
                n++;
            } else {
                EOS++;
            }
        }

        if (EOS == PAR_M) {
            done = true;
        }

        c++;
        if (c == PAR_M) {
            c = 0;
        }
    }

    N[0] = n;
}

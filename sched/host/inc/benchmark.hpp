#pragma once

#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <math.h>


#include "common.hpp"
#include "utils.hpp"
#include "ocl.hpp"
#include "buffers.hpp"

#define PAR_G           2
#define PAR_S           2

struct Benchmark
{
    size_t iterations;
    size_t batch_size;

    std::string benchmark_name;
    std::string aocx_filename;
    std::vector< std::string > kernel_names;

    Benchmark(const size_t iterations, const size_t batch_size)
    : iterations(iterations)
    , batch_size(batch_size)
    {}

    void prepare()
    {
        benchmark_name = "all2all";
        size_t total_items = iterations * batch_size * 2;
        double mem_batch = (batch_size * 2) * sizeof(float) / (double)(1 << 20);
        double mem_usage = 2 * PAR_S * mem_batch;
        std::cout << COUT_HEADER << "Iterations: "   << COUT_INTEGER << iterations  << "\n"
                  << COUT_HEADER << "Batch Items: "  << COUT_INTEGER << batch_size  << " items\n"
                  << COUT_HEADER << "Total Items: "  << COUT_INTEGER << total_items << " items\n"
                  << COUT_HEADER << "Batch Memory: " << COUT_INTEGER << mem_batch   << " MB\n"
                  << COUT_HEADER << "Memory Usage: " << COUT_INTEGER << mem_usage   << " MB\n"
                  << std::endl;

        aocx_filename = "./all2all.aocx";

        for (size_t i = 0; i < PAR_G; ++i) {
            kernel_names.push_back("generator" + std::to_string(i));
        }
        for (size_t i = 0; i < PAR_S; ++i) {
            kernel_names.push_back("sink" + std::to_string(i));
        }
    }

    void start_and_wait(int platform_id = -1, int device_id = -1)
    {
        prepare();
        const size_t k_nums = kernel_names.size();

        std::cout << COUT_HEADER << "Name: "      << benchmark_name << "\n"
                  << COUT_HEADER << "Bitstream: " << aocx_filename
                  << std::endl;

        // OpenCL init
        OCL ocl;
        ocl.init(aocx_filename, platform_id, device_id, true);

         // Queues
        std::vector<cl_command_queue> queues(k_nums);
        for (size_t i = 0; i < k_nums; ++i) {
            queues[i] = ocl.createCommandQueue();
        }

         // Buffer
        clMemory<float> * dst[PAR_S];
        clMemory<cl_uint> * N[PAR_S];
        for (size_t i = 0; i < PAR_S; ++i) {
            dst[i] = new clMemShared<float>(ocl.context, queues[PAR_G + i], 2 * batch_size, CL_MEM_WRITE_ONLY);
            dst[i]->map(CL_MAP_READ);

            N[i] = new clMemShared<cl_uint>(ocl.context, queues[PAR_G + i], 1, CL_MEM_WRITE_ONLY);
            N[i]->map(CL_MAP_READ);
        }

        // Kernels
        std::vector<cl_kernel> kernels(k_nums);
        for (size_t i = 0; i < k_nums; ++i) {
            kernels[i] = ocl.createKernel(kernel_names[i]);
        }

        cl_uint size = batch_size;

        cl_uint argi;
        // source kernels
        for (size_t i = 0; i < PAR_G; ++i) {
            argi = 0;
            clCheckError(clSetKernelArg(kernels[i], argi++, sizeof(size), &size));
        }

        // sink kernels
        for (size_t i = 0; i < PAR_S; ++i) {
            argi = 0;
            clCheckError(clSetKernelArg(kernels[PAR_G + i], argi++, sizeof(dst[i]->buffer), &(dst[i]->buffer)));
            clCheckError(clSetKernelArg(kernels[PAR_G + i], argi++, sizeof(N[i]->buffer), &(N[i]->buffer)));
        }


        // Benchmark
        size_t gws[3] = {1, 1, 1};
        size_t lws[3] = {1, 1, 1};

        volatile cl_ulong time_start = current_time_ns();
        for (size_t i = 0; i < iterations; ++i) {
            for (size_t i = 0; i < k_nums; ++i) {
                clCheckError(clEnqueueNDRangeKernel(queues[i], kernels[i],
                                                    1, NULL, gws, lws,
                                                    0, NULL, NULL));
            }
        }
        for (size_t i = 0; i < k_nums; ++i) clFinish(queues[i]);
        volatile cl_ulong time_end = current_time_ns();

        double elapsed_time = (time_end - time_start) / 1.0e9;
        double throughput = (iterations * batch_size * 2) / elapsed_time;
        std::cout << COUT_HEADER << "Elapsed Time: " << COUT_FLOAT   << elapsed_time    << " s\n"
                  << COUT_HEADER << "Throughput: "   << COUT_INTEGER << (int)throughput << " tuples/second\n";

        for (size_t i = 0; i < PAR_S; ++i) {
            std::cout << COUT_HEADER << "Sink:" << COUT_INTEGER << N[i]->ptr[0] << " items\n";
        }
        std::cout << std::endl;

        // Releases
        for (size_t i = 0; i < PAR_S; ++i) {
            if (dst[i]) {
                dst[i]->release();
                delete dst[i];
            }

            if (N[i]) {
                N[i]->release();
                delete N[i];
            }
        }

        for (size_t i = 0; i < k_nums; ++i) if (kernels[i]) clReleaseKernel(kernels[i]);
        for (size_t i = 0; i < k_nums; ++i) if (queues[i]) clReleaseCommandQueue(queues[i]);

        ocl.clean();
    }
};

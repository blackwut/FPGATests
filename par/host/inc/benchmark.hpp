#pragma once

#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <math.h>


#include "common.hpp"
#include "utils.hpp"
#include "ocl.hpp"
#include "buffers.hpp"
#include "tuples.hpp"


enum K_TYPE {
    K_BASE,
    K_UNROLL,
    K_REPLICA,
    K_NDRANGE,
    K_FUSE
};

struct Benchmark
{
    K_TYPE kernel_type;
    size_t parallelism;
    size_t iterations;
    size_t batch_size;

    std::string benchmark_name;
    std::string aocx_filename;
    std::vector< std::string > kernel_names;

    Benchmark(const K_TYPE kernel_type, const size_t parallelism,
              const size_t iterations, const size_t batch_size)
    : kernel_type(kernel_type)
    , parallelism(parallelism)
    , iterations(iterations)
    , batch_size(batch_size)
    {}

    void prepare()
    {
        if (batch_size % parallelism) {
            std::cout << "Items per iteration has been rounded to be a multiple of parallelism: " << parallelism << "\n";
            batch_size = round_up(batch_size, parallelism);
        }

        size_t total_items = iterations * batch_size;
        double mem_batch = batch_size * sizeof(tuple_t) / (double)(1 << 20);
        double mem_usage = 2 * mem_batch;
        std::cout << COUT_HEADER << "Iterations: "   << COUT_INTEGER << iterations  << "\n"
                  << COUT_HEADER << "Batch Items: "  << COUT_INTEGER << batch_size  << " items\n"
                  << COUT_HEADER << "Total Items: "  << COUT_INTEGER << total_items << " items\n"
                  << COUT_HEADER << "Batch Memory: " << COUT_INTEGER << mem_batch   << " MB\n"
                  << COUT_HEADER << "Memory Usage: " << COUT_INTEGER << mem_usage   << " MB\n"
                  << std::endl;

        // aocx_filename
        switch (kernel_type) {
            case K_BASE:    benchmark_name = "base";    break;
            case K_UNROLL:  benchmark_name = "unroll";  break;
            case K_REPLICA: benchmark_name = "replica"; break;
            case K_NDRANGE: benchmark_name = "ndrange"; break;
            case K_FUSE:    benchmark_name = "fuse";    break;
        }

        if (kernel_type != K_TYPE::K_BASE) {
            aocx_filename = "./" + benchmark_name + "_" + std::to_string(parallelism) + ".aocx";
        } else {
            aocx_filename = "./" + benchmark_name + ".aocx";
        }


        // kernel_names
        switch (kernel_type) {
            case K_BASE:
            case K_UNROLL:
            case K_NDRANGE:
            case K_FUSE:
                kernel_names.push_back("source");
                kernel_names.push_back("map");
                kernel_names.push_back("sink");
                break;
            case K_REPLICA:
                kernel_names.push_back("source");
                for (size_t p = 0; p < parallelism; ++p) {
                    kernel_names.push_back("map" + std::to_string(p));
                }
                kernel_names.push_back("sink");
                break;
        }
    }

    void fill_dataset(tuple_t * dataset, const size_t N)
    {
        for (size_t n = 0; n < N; ++n) {
            for (size_t i = 0; i < TUPLE_SIZE; ++i) {
                dataset[n].values[i] = next_float();
            }
        }
    }

    bool check_results(const tuple_t * dataset, const tuple_t * results, size_t N)
    {
        for (size_t n = 0; n < N; ++n) {
            for (size_t i = 0; i < TUPLE_SIZE; ++i) {
                if (!approximatelyEqual(sin(dataset[n].values[i]), results[n].values[i])) {
                    std::cout << "tuple[" << n << "].values[" << i << "] mismatch!!!" << std::endl;
                    return false;
                }
            }
        }
        return true;
    }

    void start_and_wait(int platform_id = -1, int device_id = -1)
    {
        prepare();
        const size_t k_nums = kernel_names.size();

        std::cout << COUT_HEADER << "Name: " << benchmark_name << "\n"
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

         // Buffers
        clMemory<tuple_t> * src = new clMemShared<tuple_t>(ocl.context, queues[0],          batch_size, CL_MEM_READ_ONLY);
        clMemory<tuple_t> * dst = new clMemShared<tuple_t>(ocl.context, queues[k_nums - 1], batch_size, CL_MEM_WRITE_ONLY);
        src->map(CL_MAP_WRITE);
        dst->map(CL_MAP_READ);

        fill_dataset(src->ptr, batch_size);

        // Kernels
        std::vector<cl_kernel> kernels(k_nums);
        for (size_t i = 0; i < k_nums; ++i) {
            kernels[i] = ocl.createKernel(kernel_names[i]);
        }

        cl_uint size = batch_size;
        if (kernel_type != K_TYPE::K_BASE) {
            size /= parallelism;
        }

        cl_uint argi = 0;
        // source kernel
        clCheckError(clSetKernelArg(kernels[0], argi++, sizeof(src->buffer), &src->buffer));
        clCheckError(clSetKernelArg(kernels[0], argi++, sizeof(size),  &size));

        // map kernels
        for (size_t i = 1; i < k_nums - 1; ++i) {
            clCheckError(clSetKernelArg(kernels[i], 0, sizeof(size),  &size));
        }

        // sink kernel
        argi = 0;
        clCheckError(clSetKernelArg(kernels[k_nums - 1], argi++, sizeof(dst->buffer), &dst->buffer));
        clCheckError(clSetKernelArg(kernels[k_nums - 1], argi++, sizeof(size),  &size));


        // Benchmark
        size_t gws[3] = {1, 1, 1};
        size_t lws[3] = {1, 1, 1};
        if (kernel_type == K_TYPE::K_NDRANGE) {
            gws[0] = parallelism;
            lws[0] = parallelism;
        }

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

        if (!check_results(src->ptr, dst->ptr, batch_size)) {
            std::cout << "ATTENTION!!! CHECKING RESULTS FAILED!!! " << std::endl;
        }

        double elapsed_time = (time_end - time_start) / 1.0e9;
        double throughput = (iterations * batch_size) / elapsed_time;
        std::cout << COUT_HEADER << "Elapsed Time: " << COUT_FLOAT   << elapsed_time    << " s\n"
                  << COUT_HEADER << "Throughput: "   << COUT_INTEGER << (int)throughput << " tuples/second\n"
                  << std::endl;

        // Releases
        src->release();
        dst->release();

        delete src;
        delete dst;

        for (size_t i = 0; i < k_nums; ++i) if (kernels[i]) clReleaseKernel(kernels[i]);
        for (size_t i = 0; i < k_nums; ++i) if (queues[i]) clReleaseCommandQueue(queues[i]);

        ocl.clean();
    }
};

#pragma once

#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <math.h>


#include "common.hpp"
#include "utils.hpp"
#include "ocl.hpp"

using namespace std;

struct FPGA
{
    void start_and_wait(const size_t workers, const size_t size,
                        const int platform_id = -1, const int device_id = -1)
    {
        cout << "FPGA Test started!" << endl;

        vector< string> kernel_names;
        string aocx_filename = "farm_" + to_string(workers) + ".aocx";
        
        kernel_names.push_back("emitter");
        for (size_t i = 0; i < workers; ++i) {
            kernel_names.push_back("worker" + to_string(i));
        }
        kernel_names.push_back("sink");
        const size_t k_nums = kernel_names.size();

        // OpenCL init
        OCL ocl;
        ocl.init(aocx_filename, platform_id, device_id, true);

         // Queues
        vector<cl_command_queue> queues(k_nums);
        for (size_t i = 0; i < k_nums; ++i) {
            queues[i] = ocl.createCommandQueue();
        }


        cl_uint argi;                               // argument index
        cl_uint ec_size = round_up(size, workers);  // emitter/collector size
        cl_uint w_size = u_size / workers;          // worker size

        argi = 0;
        clCheckError(clSetKernelArg(kernels[i], argi++, sizeof(ec_size), &ec_size));

        for (size_t i = 0; i < workers; ++i) {
            argi = 0;
            clCheckError(clSetKernelArg(kernels[i + 1], argi++, sizeof(w_size), &w_size));
        }

        // sink kernels
        argi = 0;
        clCheckError(clSetKernelArg(kernels[workers + 1], argi++, sizeof(ec_size), &ec_size));


        // Benchmark
        size_t gws[3] = {1, 1, 1};
        size_t lws[3] = {1, 1, 1};

        volatile cl_ulong time_start = current_time_ns();
        for (size_t i = 0; i < k_nums; ++i) {
            clCheckError(clEnqueueNDRangeKernel(queues[i], kernels[i],
                                                1, NULL, gws, lws,
                                                0, NULL, NULL));
        }
        for (size_t i = 0; i < k_nums; ++i) clFinish(queues[i]);
        volatile cl_ulong time_end = current_time_ns();

        double elapsed_time = (time_end - time_start) / 1.0e9;
        double throughput = size / elapsed_time;
        cout << COUT_HEADER << "Items: "        << COUT_INTEGER << ec_size         << "\n"
             << COUT_HEADER << "Elapsed Time: " << COUT_FLOAT   << elapsed_time    << " s\n"
             << COUT_HEADER << "Throughput: "   << COUT_INTEGER << (int)throughput << " incr/s\n"
             << endl;

        // Releases
        for (size_t i = 0; i < k_nums; ++i) if (kernels[i]) clReleaseKernel(kernels[i]);
        for (size_t i = 0; i < k_nums; ++i) if (queues[i]) clReleaseCommandQueue(queues[i]);

        ocl.clean();
    }
};

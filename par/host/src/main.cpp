#include <iostream>
#include <iomanip>
#include <utility>
#include <stdlib.h>
#include <math.h>

#include "opencl.hpp"
#include "common.hpp"
#include "options.hpp"
#include "buffers.hpp"
#include "utils.hpp"
#include "tuples.hpp"

using namespace std;

struct OCL
{
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_program program;

    void init(const std::string filename, int platformid = -1, int deviceid = -1) {
        platform = (platformid < 0) ? clPromptPlatform() : clSelectPlatform(platformid);
        device = (deviceid < 0) ? clPromptDevice(platform) : clSelectDevice(platform, deviceid);
        context = clCreateContextFor(platform, device);
        program = clCreateBuildProgramFromBinary(context, device, filename);
    }

    cl_command_queue createCommandQueue() {
        cl_int status;
        cl_command_queue queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &status);
        clCheckErrorMsg(status, "Failed to create command queue");
        return queue;
    }

    cl_kernel createKernel(const char * kernel_name) {
        cl_int status;
        cl_kernel kernel = clCreateKernel(program, kernel_name, &status);
        clCheckErrorMsg(status, "Failed to create kernel");
        return kernel;
    }

    void clean() {
        if (program) clReleaseProgram(program);
        if (context) clReleaseContext(context);
    }
};

// creates tuples and returns the total number of spikes
void fill_dataset(tuple_t * dataset, const size_t N)
{
    for (size_t n = 0; n < N; ++n) {
        random_fill(dataset[n].values, TUPLE_SIZE);
    }
}

void benchmark(OCL & ocl,
               const K_TYPE k_type,         // kernel type
               const size_t k_nums,         // number of kernels
               const char * k_names[],      // kernel names
               const size_t iterations,
               const size_t size)
{
     // Queues
    cl_command_queue * queues = new cl_command_queue[k_nums];
    for (int i = 0; i < k_nums; ++i) {
        queues[i] = ocl.createCommandQueue();
    }

     // Buffers
    clMemory<tuple_t> * src;
    clMemory<tuple_t> * dst;

    src = new clMemShared<tuple_t>(ocl.context, queues[0], size, CL_MEM_READ_ONLY);
    dst = new clMemShared<tuple_t>(ocl.context, queues[k_nums - 1], size, CL_MEM_WRITE_ONLY);
    src->map(CL_MAP_WRITE);
    dst->map(CL_MAP_READ);

    fill_dataset(src->ptr, size);

    // Kernels
    cl_kernel * kernels = new cl_kernel[k_nums];
    for (int i = 0; i < k_nums; ++i) {
        kernels[i] = ocl.createKernel(k_names[i]);
    }

    cl_uint batch_size = size;
    if (k_type != K_TYPE::K_BASE) {
        batch_size /= COMPUTE_UNITS;
    }

    cl_uint argi = 0;
    // source kernel
    clCheckError(clSetKernelArg(kernels[0], argi++, sizeof(src->buffer), &src->buffer));
    clCheckError(clSetKernelArg(kernels[0], argi++, sizeof(batch_size),  &batch_size));

    // map kernels
    for (int i = 0; i < k_nums - 2; ++i) {
        clCheckError(clSetKernelArg(kernels[i + 1], 0, sizeof(batch_size),  &batch_size));
    }

    // sink kernel
    argi = 0;
    clCheckError(clSetKernelArg(kernels[k_nums - 1], argi++, sizeof(dst->buffer), &dst->buffer));
    clCheckError(clSetKernelArg(kernels[k_nums - 1], argi++, sizeof(batch_size),  &batch_size));


    // Benchmark
    size_t gws[3] = {1, 1, 1};
    size_t lws[3] = {1, 1, 1};
    if (k_type == K_TYPE::K_NDRANGE) {
        gws[0] = COMPUTE_UNITS;
        lws[0] = COMPUTE_UNITS;
    }

    volatile cl_ulong time_start = current_time_ns();
    for (int i = 0; i < iterations; ++i) {
        for (int i = 0; i < k_nums; ++i) {
            clCheckError(clEnqueueNDRangeKernel(queues[i], kernels[i],
                                                1, NULL, gws, lws,
                                                0, NULL, NULL));
        }
    }
    for (int i = 0; i < k_nums; ++i) clFinish(queues[i]);
    volatile cl_ulong time_end = current_time_ns();

    double elapsed_time = (time_end - time_start) / 1.0e9;
    double throughput = (iterations * size) / elapsed_time;
    cout << "       Elapsed Time: " << setw(4) << fixed << elapsed_time << " s\n";
    cout << "Measured throughput: " << (int) throughput << " tuples/second\n\n";

    // Releases
    src->release();
    dst->release();

    delete src;
    delete dst;

    for (int i = 0; i < k_nums; ++i) if (kernels[i]) clReleaseKernel(kernels[i]);
    for (int i = 0; i < k_nums; ++i) if (queues[i]) clReleaseCommandQueue(queues[i]);

    delete[] kernels;
    delete[] queues;
}

void benchmark_replica_new(OCL & ocl,
                           const K_TYPE k_type,         // kernel type
                           const size_t k_nums,         // number of kernels
                           const char * k_names[],      // kernel names
                           const size_t iterations,
                           const size_t size)
{
     // Queues
    cl_command_queue * queues = new cl_command_queue[k_nums];
    for (int i = 0; i < k_nums; ++i) {
        queues[i] = ocl.createCommandQueue();
    }

     // Buffers
    clMemory<tuple_t> * src;
    clMemory<tuple_t> * dst;

    src = new clMemShared<tuple_t>(ocl.context, queues[0], size, CL_MEM_READ_ONLY);
    dst = new clMemShared<tuple_t>(ocl.context, queues[k_nums - 1], size, CL_MEM_WRITE_ONLY);
    src->map(CL_MAP_WRITE);
    dst->map(CL_MAP_READ);

    fill_dataset(src->ptr, size);

    // Kernels
    cl_kernel * kernels = new cl_kernel[k_nums];
    for (int i = 0; i < k_nums; ++i) {
        kernels[i] = ocl.createKernel(k_names[i]);
    }

    cl_uint batch_size = size / COMPUTE_UNITS;

    cl_uint argi;
    // source kernel
    for (int i = 0; i < COMPUTE_UNITS; ++i) {
        argi = 0;
        clCheckError(clSetKernelArg(kernels[i], argi++, sizeof(src->buffer), &src->buffer));
        clCheckError(clSetKernelArg(kernels[i], argi++, sizeof(batch_size),  &batch_size));
    }

    // map kernels
    for (int i = 0; i < COMPUTE_UNITS; ++i) {
        clCheckError(clSetKernelArg(kernels[i + COMPUTE_UNITS], 0, sizeof(batch_size),  &batch_size));
    }

    // sink kernel
    for (int i = 0; i < COMPUTE_UNITS; ++i) {
        argi = 0;
        clCheckError(clSetKernelArg(kernels[i + COMPUTE_UNITS * 2], argi++, sizeof(dst->buffer), &dst->buffer));
        clCheckError(clSetKernelArg(kernels[i + COMPUTE_UNITS * 2], argi++, sizeof(batch_size),  &batch_size));
    }


    // Benchmark
    size_t gws[3] = {1, 1, 1};
    size_t lws[3] = {1, 1, 1};

    volatile cl_ulong time_start = current_time_ns();
    for (int i = 0; i < iterations; ++i) {
        for (int i = 0; i < k_nums; ++i) {
            clCheckError(clEnqueueNDRangeKernel(queues[i], kernels[i],
                                                1, NULL, gws, lws,
                                                0, NULL, NULL));
        }
    }
    for (int i = 0; i < k_nums; ++i) clFinish(queues[i]);
    volatile cl_ulong time_end = current_time_ns();

    double elapsed_time = (time_end - time_start) / 1.0e9;
    double throughput = (iterations * size) / elapsed_time;
    cout << "       Elapsed Time: " << setw(4) << fixed << elapsed_time << " s\n";
    cout << "Measured throughput: " << (int) throughput << " tuples/second\n\n";

    // Releases
    src->release();
    dst->release();

    delete src;
    delete dst;

    for (int i = 0; i < k_nums; ++i) if (kernels[i]) clReleaseKernel(kernels[i]);
    for (int i = 0; i < k_nums; ++i) if (queues[i]) clReleaseCommandQueue(queues[i]);

    delete[] kernels;
    delete[] queues;
}

int main(int argc, char * argv[])
{
    Options opt;
    opt.process_args(argc, argv);

    if (opt.size % COMPUTE_UNITS) {
        cout << "Items per iteration has been rounded to be a multiple of " << COMPUTE_UNITS << "\n";
        opt.size = ((opt.size + COMPUTE_UNITS - 1) / COMPUTE_UNITS) * COMPUTE_UNITS;
    }

    cl_uint total_items = (cl_uint)opt.iterations * opt.size;
    double mem_batch = opt.size * sizeof(tuple_t) / (double)(1 << 20);
    double mem_total = opt.iterations * mem_batch;
    cout << fixed << setprecision(3)
         << "   Iterations: " << opt.iterations << "\n"
         << "  Batch Items: " << opt.size       << " items\n"
         << " Batch Memory: " << mem_batch      << " MB\n"
         << "  Total Items: " << total_items    << " items\n"
         << " Total Memory: " << mem_total      << " MB\n"
         << "\n";

    volatile cl_ulong time_start;
    volatile cl_ulong time_end;

    if (opt.k_base) {
        OCL ocl;
        time_start = current_time_ns();
        ocl.init(P_BASE_FILENAME, opt.platform, opt.device);
        time_end = current_time_ns();
        cout << "    Loading Bitstream: " << P_BASE_FILENAME << endl;
        cout << "       BASE init took: " << right << setw(6) << fixed << (time_end - time_start) * 1.0e-9 << " s\n";
        benchmark(ocl,
                  K_TYPE::K_BASE, K_BASE_NUMS, K_BASE_NAMES,
                  opt.iterations, opt.size);
        ocl.clean();
    }

    if (opt.k_unroll) {
        OCL ocl;
        time_start = current_time_ns();
        ocl.init(P_UNROLL_FILENAME, opt.platform, opt.device);
        time_end = current_time_ns();
        cout << "    Loading Bitstream: " << P_UNROLL_FILENAME << endl;
        cout << "     UNROLL init took: " << right << setw(6) << fixed << (time_end - time_start) * 1.0e-9 << " s\n";
        benchmark(ocl,
                  K_TYPE::K_UNROLL, K_UNROLL_NUMS, K_UNROLL_NAMES,
                  opt.iterations, opt.size);
        ocl.clean();
    }

    if (opt.k_replica) {
        OCL ocl;
        time_start = current_time_ns();
        ocl.init(P_REPLICA_FILENAME, opt.platform, opt.device);
        time_end = current_time_ns();
        cout << "    Loading Bitstream: " << P_REPLICA_FILENAME << endl;
        cout << "    REPLICA init took: " << right << setw(6) << fixed << (time_end - time_start) * 1.0e-9 << " s\n";
        benchmark(ocl,
                  K_TYPE::K_REPLICA, K_REPLICA_NUMS, K_REPLICA_NAMES,
                  opt.iterations, opt.size);
        ocl.clean();
    }

    if (opt.k_replica_new) {
        OCL ocl;
        time_start = current_time_ns();
        ocl.init(P_REPLICA_NEW_FILENAME, opt.platform, opt.device);
        time_end = current_time_ns();
        cout << "    Loading Bitstream: " << P_REPLICA_NEW_FILENAME << endl;
        cout << "REPLICA_NEW init took: " << right << setw(6) << fixed << (time_end - time_start) * 1.0e-9 << " s\n";
        benchmark_replica_new(ocl,
                              K_TYPE::K_REPLICA_NEW, K_REPLICA_NEW_NUMS, K_REPLICA_NEW_NAMES,
                              opt.iterations, opt.size);
        ocl.clean();
    }

    if (opt.k_ndrange) {
        OCL ocl;
        time_start = current_time_ns();
        ocl.init(P_NDRANGE_FILENAME, opt.platform, opt.device);
        time_end = current_time_ns();
        cout << "    Loading Bitstream: " << P_NDRANGE_FILENAME << endl;
        cout << "    NDRANGE init took: " << right << setw(6) << fixed << (time_end - time_start) * 1.0e-9 << " s\n";
        benchmark(ocl,
                  K_TYPE::K_NDRANGE, K_NDRANGE_NUMS, K_NDRANGE_NAMES,
                  opt.iterations, opt.size);
        ocl.clean();
    }

    return 0;
}
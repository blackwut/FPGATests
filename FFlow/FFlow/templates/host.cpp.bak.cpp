#include <iostream>
#include <queue>

#include "ocl.hpp"
#include "buffers.hpp"
#include "utils.hpp"
#include "tuples.h"

{% set src_data_type = nodes[0].o_channel.data_type %}

{% for node in nodes %}
#define {{ node.declare_macro_par() }}
{% endfor %}

void fill_dataset(std::vector<{{ src_data_type }}> & dataset, const size_t N)
{
    for (size_t n = 0; n < N; ++n) {
        dataset[n].value = next_float();
    }
}

struct FSource
{

    OCL ocl;

    cl_command_queue source_queue;
    cl_kernel source_kernel;
    std::queue<cl_event> source_events; // front() = the first element to pop()

    size_t max_batch_size;
    size_t number_of_buffers;
    size_t batch_number;
    std::vector< clMemory<{{ src_data_type }}> *> src_buffers;
    std::vector<cl_command_queue> queues;


    FSource(OCL & ocl,
            cl_uint max_batch_size,
            size_t number_of_buffers)
    : ocl(ocl)
    , max_batch_size(max_batch_size)
    , number_of_buffers(number_of_buffers)
    , batch_number(0)
    , src_buffers(number_of_buffers)
    , queues(number_of_buffers)
    {
        source_queue = ocl.createCommandQueue();
        source_kernel = ocl.createKernel("{{ nodes[0].name }}_0");

        for (size_t i = 0; i < number_of_buffers; ++i) {
            queues[i] = ocl.createCommandQueue();
            src_buffers[i] = new clMemShared<{{ src_data_type }}>(ocl.context, queues[i], max_batch_size, CL_MEM_READ_ONLY);
            src_buffers[i]->map(CL_MAP_WRITE);
        }
    }

    void push(const std::vector<{{ src_data_type }}> & batch,
              cl_uint batch_size,
              bool last = false)
    {

        if (source_events.size() == number_of_buffers) {
            cl_event evt = source_events.front();

            cl_int event_status;
            // TODO: busy waiting!!! insert a delay maybe?
            do {
                clCheckError(clGetEventInfo(evt, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(cl_int), &event_status, NULL));
            } while (event_status != CL_COMPLETE);

            clReleaseEvent(evt);
            source_events.pop();
        }

        clMemory<{{ src_data_type }}> * buffer = src_buffers[batch_number];

        // copy to device (using shared buffer)
        for (size_t i = 0; i < batch_size; ++i) {
            buffer->ptr[i] = batch[i];
        }

        cl_uint last_push = (cl_uint)1 * last;

        cl_uint argi = 0;
        clCheckError(clSetKernelArg(source_kernel, argi++, sizeof(buffer), &buffer->buffer));
        clCheckError(clSetKernelArg(source_kernel, argi++, sizeof(batch_size), &batch_size));
        clCheckError(clSetKernelArg(source_kernel, argi++, sizeof(last_push), &last_push));

        // enqueue source kernel
        cl_event evt;
        const size_t gws[3] = {1, 1, 1};
        const size_t lws[3] = {1, 1, 1};
        clCheckError(clEnqueueNDRangeKernel(source_queue, source_kernel,
                                            1, NULL, gws, lws,
                                            0, NULL, &evt));

        source_events.push(evt);
    }

    void finish()
    {
        clFinish(source_queue);
    }

    void clean()
    {

        for (auto & e : source_events) {
            clReleaseEvent(e);
        }

        for (auto & q : queues) {
            clFinish(q);
        }

        for (auto b : src_buffers) {
            b->release();
        }

        if (source_kernel) clReleaseKernel(source_kernel);
        if (source_queue) clReleaseCommandQueue(source_queue);
        for (auto & q : queues) if (q) clReleaseCommandQueue(q);
    }

    // void gne() {
    //     cl_event dependencies[2];
    //     for (int i=0; i<MAX_ITERATIONS; i++) {
    //     if (i < 2) {
    //         clEnqueueWriteBuffer(writeQ, inputBufferD[i % 2], CL_FALSE, 0, 0, inputBufferH[i], 0, NULL, &writeEvent[i])
    //         clFlush(writeQ);
    //         clSetKernelArg(kernel, 0, sizeof(cl_mem *), &inputBufferD[i % 2]);
    //         clEnqueueNDRangeKernel(kernelQ, kernel,  ...,  1,  &writeEvent[i], &kernelEvent[i]);
    //         clFlush(kernelQ);
    //     } else {
    //         clEnqueueWriteBuffer(writeQ, inputBufferD[i % 2], CL_FALSE, 0, 0, inputBufferH[i], 1, &kernelEvent[i-2], &writeEvent[i]);
    //         clFlush(writeQ);
    //         dependencies[0] = writeEvent[i];
    //         dependencies[1] = readEvent[i - 2];
    //         clSetKernelArg(kernel, 0, sizeof(cl_mem *), &inputBufferD[i % 2]);
    //         clEnqueueNDRangeKernel(kernelQ,  kernel,  ...,  2,  dependencies, &kernelEvent[i]);
    //         clFlush(kernelQ);
    //     }
    // }
};

void start_and_wait(std::string & aocx_filename, cl_uint batch_size, int platform_id = -1, int device_id = -1)
{
    // KERNEL NAMES
    std::vector< std::string > kernel_names;
    {% for node in nodes %}
    // {{ node.name }} kernel
    for (size_t i = 1; i < {{ node.get_macro_par_name() }}; ++i) {
        kernel_names.push_back("{{ node.name }}_" + std::to_string(i));
    }
    {% endfor %}

    // total number of nodes (replicas)
    const size_t k_nums = kernel_names.size();

    // OpenCL init
    OCL ocl;
    ocl.init(aocx_filename, platform_id, device_id, true);

     // Queues
    std::vector<cl_command_queue> queues(k_nums);
    for (size_t i = 1; i < k_nums; ++i) {
        queues[i] = ocl.createCommandQueue();
    }

    // Kernels
    std::vector<cl_kernel> kernels(k_nums);
    for (size_t i = 1; i < k_nums; ++i) {
        kernels[i] = ocl.createKernel(kernel_names[i]);
    }

    // clMemory<{{ src_data_type }}> * src = NULL;
    // src = new clMemShared<{{ src_data_type }}>(ocl.context, queues[0], batch_size, CL_MEM_READ_ONLY);
    // src->map(CL_MAP_WRITE);

    // fill_dataset(src->ptr, batch_size);

    // cl_uint shutdown = 1;

{#
    // Buffers
    {% for node in nodes if node.buffers|lenght > 0 %}
    for (size_t i = 0; i < {{ node.get_macro_par_name() }}; ++i) {
        {% for b in node.buffers %}
        clMemory<{{b.data_type}}> * {{node.name + '_' + b.name}} = nullptr;
        {% endfor %}
    {% endfor %}

    {% for node in nodes if node.buffers|lenght > 0 %}
    for (size_t i = 0; i < {{ node.get_macro_par_name() }}; ++i) {
        {% for b in node.buffers %}
        {{node.name + '_' + b.name}} = new clMemShared<{{ b.data_type }}>(ocl.context, queues[i + {{ node.id }}], {{ b.size }} * sizeof({{ b.data_type }}), CL_MEM_READ_WRITE);
        {{node.name + '_' + b.name}}->map(CL_MAP_READ | CL_MAP_WRITE);
        {% endfor %}
    {% endfor %}

    cl_uint argi = 0;
    // {{ node.name }} kernel
    {% for node in nodes if node.buffers|lenght > 0 %}
    for (size_t i = 0; i < {{ node.get_macro_par_name() }}; ++i) {
        argi = 0;
        {% for b in node.buffers %}
        clCheckError(clSetKernelArg(kernels[i + {{ node.id }}], argi++, sizeof(src->buffer), &src->buffer));
        {% endfor %}
    {% endfor %}
#}

    // cl_uint argi = 0;
    // clCheckError(clSetKernelArg(kernels[0], argi++, sizeof(src->buffer), &src->buffer));
    // clCheckError(clSetKernelArg(kernels[0], argi++, sizeof(batch_size),  &batch_size));
    // clCheckError(clSetKernelArg(kernels[0], argi++, sizeof(shutdown), &shutdown));


    // Run FPipeGraph
    const size_t gws[3] = {1, 1, 1};
    const size_t lws[3] = {1, 1, 1};

    volatile cl_ulong time_start = current_time_ns();
    for (size_t i = 1; i < k_nums; ++i) {
        clCheckError(clEnqueueNDRangeKernel(queues[i], kernels[i],
                                            1, NULL, gws, lws,
                                            0, NULL, NULL));
    }

    size_t dataset_size = 512;
    std::vector< {{ src_data_type }} > dataset(dataset_size);
    fill_dataset(dataset, dataset_size);

    FSource source_node(ocl, 1024, 2);
    for (size_t i = 0; i < 3; ++i) {
        source_node.push(dataset, dataset_size, i == 2);
    }

    source_node.finish();
    for (size_t i = 0; i < k_nums; ++i) clFinish(queues[i]);
    volatile cl_ulong time_end = current_time_ns();

    // Print results
    double elapsed_time = (time_end - time_start) / 1.0e9;
    std::cout << COUT_HEADER << "Elapsed Time: " << COUT_FLOAT   << elapsed_time    << " s\n"
              << std::endl;

    // Releases
    for (size_t i = 1; i < k_nums; ++i) if (kernels[i]) clReleaseKernel(kernels[i]);
    for (size_t i = 1; i < k_nums; ++i) if (queues[i]) clReleaseCommandQueue(queues[i]);

    ocl.clean();
}

int main(int argc, char * argv[])
{
    std::string aocx_filename = "device.aocx";
    int platform_id = 1;
    int device_id = 0;

    if (argc > 1) platform_id = atoi(argv[1]);
    if (argc > 2) device_id = atoi(argv[2]);

    std::cout << "platform_id: " << platform_id << std::endl;
    std::cout << "  device_id: " << device_id   << std::endl;

    start_and_wait(aocx_filename, 1024, platform_id, device_id);

    return 0;
}


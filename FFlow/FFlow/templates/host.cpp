#include <iostream>
#include <vector>
#include <thread>

#include "ocl.hpp"
#include "utils.hpp"
#include "tuples.h"

/* Blocking QUEUE
#include <mutex>
#include <condition_variable>
#include <deque>

template <typename T>
class queue
{
private:
    std::mutex              d_mutex;
    std::condition_variable d_condition;
    std::deque<T>           d_queue;
public:
    void push(T const& value) {
        {
            std::unique_lock<std::mutex> lock(this->d_mutex);
            d_queue.push_front(value);
        }
        this->d_condition.notify_one();
    }
    T pop() {
        std::unique_lock<std::mutex> lock(this->d_mutex);
        this->d_condition.wait(lock, [=]{ return !this->d_queue.empty(); });
        T rc(std::move(this->d_queue.back()));
        this->d_queue.pop_back();
        return rc;
    }
};
END Blocking QUEUE*/


{% set source_data_type = source.o_channel.data_type %}
{% set sink_data_type = sink.i_channel.data_type %}

{% for node in nodes %}
#define {{ node.declare_macro_par() }}
{% endfor %}

void fill_dataset(std::vector<{{ source_data_type }}> & dataset, const size_t N)
{
    for (size_t n = 0; n < N; ++n) {
        dataset[n].key = n;
        dataset[n].value = (float)n;
    }
}

bool check_results(std::vector<{{ sink_data_type }}> & results, const size_t N)
{
    sort(results.begin(), results.end(), [](const {{ sink_data_type }} & a, const {{ sink_data_type }} & b){
        return a.key < b.key;
    });

    for (size_t i = 0; i < N; ++i) {
        if (!approximatelyEqual((float)i + 2.0f, results[i].value)) {
            return false;
        }
    }
    return true;
}

struct FSource
{
    // TODO: use a Blocking Queue?
    OCL & ocl;

    cl_uint max_batch_size;
    size_t number_of_buffers;
    size_t iteration;
    cl_command_queue buffers_queue;
    std::vector<cl_mem> buffers;
    std::vector<cl_event> buffers_events;

    cl_command_queue source_queue;
    cl_kernel source_kernel;
    std::vector<cl_event> source_events;

    FSource(OCL & ocl,
            cl_uint max_batch_size,
            size_t number_of_buffers)
    : ocl(ocl)
    , max_batch_size(max_batch_size)
    , number_of_buffers(number_of_buffers)
    , iteration(0)
    , buffers(number_of_buffers)
    , buffers_events(number_of_buffers)
    , source_events(number_of_buffers)
    {
        buffers_queue = ocl.createCommandQueue();
        source_queue = ocl.createCommandQueue();
        source_kernel = ocl.createKernel("{{ source.name }}_0");

        for (size_t i = 0; i < number_of_buffers; ++i) {
            cl_int status;
            buffers[i] = clCreateBuffer(ocl.context,
                                        CL_MEM_HOST_WRITE_ONLY | CL_MEM_READ_ONLY,
                                        max_batch_size * sizeof({{ source_data_type}}),
                                        NULL, &status);
            clCheckErrorMsg(status, "Failed to create clBuffer");
        }
    }

    void push(const std::vector<{{ source_data_type }}> & batch,
              cl_uint batch_size,
              bool last = false)
    {
        cl_uint last_push = (cl_uint)1 * last;

        if (iteration < number_of_buffers) {
            // push buffer
            clCheckError(clEnqueueWriteBuffer(buffers_queue,
                                              buffers[iteration],
                                              CL_TRUE, 0,
                                              batch_size * sizeof({{ source_data_type }}), batch.data(),
                                              0, NULL, &buffers_events[iteration]));
            clFlush(buffers_queue);

            // set kernel args
            cl_uint argi = 0;
            clCheckError(clSetKernelArg(source_kernel, argi++, sizeof(buffers[iteration]), &buffers[iteration]));
            clCheckError(clSetKernelArg(source_kernel, argi++, sizeof(batch_size), &batch_size));
            clCheckError(clSetKernelArg(source_kernel, argi++, sizeof(last_push), &last_push));

            // launch kernel
            const size_t gws[3] = {1, 1, 1};
            const size_t lws[3] = {1, 1, 1};
            clCheckError(clEnqueueNDRangeKernel(source_queue, source_kernel,
                                                1, NULL, gws, lws,
                                                1, &buffers_events[iteration], &source_events[iteration]));
            clFlush(source_queue);
        } else {
            // push buffer
            clCheckError(clEnqueueWriteBuffer(buffers_queue,
                                              buffers[iteration % number_of_buffers],
                                              CL_TRUE, 0,
                                              batch_size * sizeof({{ source_data_type }}), batch.data(),
                                              1, &source_events[(iteration - number_of_buffers) % number_of_buffers], &buffers_events[iteration % number_of_buffers]));
            clFlush(buffers_queue);

            // set kernel args
            cl_uint argi = 0;
            clCheckError(clSetKernelArg(source_kernel, argi++, sizeof(buffers[iteration % number_of_buffers]), &buffers[iteration % number_of_buffers]));
            clCheckError(clSetKernelArg(source_kernel, argi++, sizeof(batch_size), &batch_size));
            clCheckError(clSetKernelArg(source_kernel, argi++, sizeof(last_push), &last_push));

            // launch kernel
            const size_t gws[3] = {1, 1, 1};
            const size_t lws[3] = {1, 1, 1};
            clCheckError(clEnqueueNDRangeKernel(source_queue, source_kernel,
                                                1, NULL, gws, lws,
                                                1, &buffers_events[iteration % number_of_buffers], &source_events[iteration % number_of_buffers]));
            clFlush(source_queue);
        }

        iteration++;
    }

    void finish()
    {
        clFinish(buffers_queue);
        clFinish(source_queue);
    }

    void clean()
    {
        finish();

        for (auto & b : buffers) {
            if (b) clCheckError(clReleaseMemObject(b));
        }
        if (buffers_queue) clReleaseCommandQueue(buffers_queue);

        if (source_kernel) clReleaseKernel(source_kernel);
        if (source_queue) clReleaseCommandQueue(source_queue);
    }
};

struct FSink
{
    OCL & ocl;

    cl_uint iteration;
    cl_uint max_batch_size;
    cl_uint last_EOS;

    cl_command_queue sink_queue;
    cl_kernel sink_kernel;
    cl_event sink_event;
    cl_mem buffer;
    cl_command_queue buffer_queue;
    cl_event buffer_event;

    cl_mem received_d;
    cl_mem shutdown_d;

    cl_command_queue received_queue;
    cl_command_queue shutdown_queue;

    cl_event received_event;
    cl_event shutdown_event;

    FSink(OCL & ocl,
            cl_uint max_batch_size)
    : ocl(ocl)
    , iteration(0)
    , max_batch_size(max_batch_size)
    , last_EOS(0)
    {
        sink_queue = ocl.createCommandQueue();
        sink_kernel = ocl.createKernel("{{ sink.name }}_0");

        buffer_queue = ocl.createCommandQueue();
        received_queue = ocl.createCommandQueue();
        shutdown_queue = ocl.createCommandQueue();

        cl_int status;
        buffer = clCreateBuffer(ocl.context,
                                CL_MEM_HOST_READ_ONLY | CL_MEM_WRITE_ONLY,
                                max_batch_size * sizeof({{ source_data_type }}),
                                NULL, &status);
        clCheckErrorMsg(status, "Failed to create buffer");
        received_d = clCreateBuffer(ocl.context,
                                    CL_MEM_HOST_READ_ONLY | CL_MEM_WRITE_ONLY,
                                    sizeof(cl_uint),
                                    NULL, &status);
        clCheckErrorMsg(status, "Failed to create received_d");
        shutdown_d = clCreateBuffer(ocl.context,
                                    CL_MEM_HOST_READ_ONLY | CL_MEM_WRITE_ONLY,
                                    sizeof(cl_uint),
                                    NULL, &status);
        clCheckErrorMsg(status, "Failed to create shutdown_d");
    }

    // return true if it is the last pop to be done
    std::vector<{{ sink_data_type }}> pop(cl_uint batch_size,
                                          cl_uint * received,
                                          bool * shutdown)
    {
        // set kernel args
        cl_uint argi = 0;
        clCheckError(clSetKernelArg(sink_kernel, argi++, sizeof(buffer), &buffer));
        clCheckError(clSetKernelArg(sink_kernel, argi++, sizeof(received_d), &received_d));
        clCheckError(clSetKernelArg(sink_kernel, argi++, sizeof(shutdown_d), &shutdown_d));
        clCheckError(clSetKernelArg(sink_kernel, argi++, sizeof(last_EOS), &last_EOS));
        clCheckError(clSetKernelArg(sink_kernel, argi++, sizeof(batch_size), &batch_size));

        // launch kernel
        const size_t gws[3] = {1, 1, 1};
        const size_t lws[3] = {1, 1, 1};
        if (iteration < 1) {
            clCheckError(clEnqueueNDRangeKernel(sink_queue, sink_kernel,
                                                1, NULL, gws, lws,
                                                0, NULL, &sink_event));
        } else {
            clCheckError(clEnqueueNDRangeKernel(sink_queue, sink_kernel,
                                                1, NULL, gws, lws,
                                                1, &buffer_event, &sink_event));
        }
        clFlush(sink_queue);
        iteration++;

        cl_uint received_h = 0;
        cl_uint shutdown_h = 0;
        clCheckError(clEnqueueReadBuffer(received_queue,
                                         received_d,
                                         CL_TRUE, 0,
                                         sizeof(cl_uint), &received_h,
                                         1, &sink_event, &received_event));

        clCheckError(clEnqueueReadBuffer(shutdown_queue,
                                         shutdown_d,
                                         CL_TRUE, 0,
                                         sizeof(cl_uint), &shutdown_h,
                                         1, &sink_event, &shutdown_event));


        std::vector<{{ sink_data_type }}> batch(received_h);

        if (received_h > 0) {
            clCheckError(clEnqueueReadBuffer(buffer_queue,
                                             buffer,
                                             CL_TRUE, 0,
                                             received_h * sizeof({{ sink_data_type }}), batch.data(),
                                             1, &sink_event, &buffer_event));
            clFlush(buffer_queue);
        }

        *received = received_h;
        last_EOS = shutdown_h;
        *shutdown = (shutdown_h == {{ sink.i_degree }});

        return batch;
    }

    void finish()
    {
        clFinish(sink_queue);
        clFinish(buffer_queue);
    }

    void clean()
    {
        finish();

        if (buffer) clCheckError(clReleaseMemObject(buffer));
        if (buffer_queue) clReleaseCommandQueue(buffer_queue);

        if (sink_kernel) clReleaseKernel(sink_kernel);
        if (sink_queue) clReleaseCommandQueue(sink_queue);
    }
};


struct FPipeGraph
{
    OCL ocl;
    size_t max_batch_size;

    size_t k_nums;
    std::vector<std::string> kernel_names;
    std::vector<cl_command_queue> queues;
    std::vector<cl_kernel> kernels;

    FSource source_node;
    FSink sink_node;

    volatile cl_ulong time_start;
    volatile cl_ulong time_stop;

    FPipeGraph(OCL & ocl,
               size_t max_batch_size,
               size_t number_of_buffers)
    : ocl(ocl)
    , max_batch_size(max_batch_size)
    , source_node(ocl, max_batch_size, number_of_buffers)
    , sink_node(ocl, max_batch_size)
    {
        // KERNEL NAMES
        {% for node in nodes %}
        // {{ node.name }} kernel
        for (size_t i = 0; i < {{ node.get_macro_par_name() }}; ++i) {
            kernel_names.push_back("{{ node.name }}_" + std::to_string(i));
        }
        {% endfor %}

        // total number of kernels ( except source(s) )
        k_nums = kernel_names.size();

        // Queues
        for (size_t i = 0; i < k_nums; ++i) {
            queues.push_back(ocl.createCommandQueue());
        }

        // Kernels
        for (size_t i = 0; i < k_nums; ++i) {
            kernels.push_back(ocl.createKernel(kernel_names[i]));
        }
    }

    void start()
    {
        // Run FPipeGraph
        const size_t gws[3] = {1, 1, 1};
        const size_t lws[3] = {1, 1, 1};

        time_start = current_time_ns();

        for (size_t i = 0; i < k_nums; ++i) {
            clCheckError(clEnqueueNDRangeKernel(queues[i], kernels[i],
                                                1, NULL, gws, lws,
                                                0, NULL, NULL));
        }
    }

    void push(const std::vector<{{ source_data_type }}> & batch,
              cl_uint batch_size,
              bool last = false)
    {
        source_node.push(batch, batch_size, last);
    }

    std::vector<{{ sink_data_type }}> pop(cl_uint batch_size,
                                          cl_uint * received,
                                          bool * shutdown)
    {
        return sink_node.pop(batch_size, received, shutdown);
    }

    void wait_and_stop()
    {
        source_node.finish();
        for (size_t i = 0; i < k_nums; ++i) clFinish(queues[i]);
        time_stop = current_time_ns();
    }

    double service_time()
    {
        return (time_stop - time_start) / 1.0e9;
    }

    void clean()
    {
        source_node.clean();
        // Releases
        for (size_t i = 1; i < k_nums; ++i) if (kernels[i]) clReleaseKernel(kernels[i]);
        for (size_t i = 1; i < k_nums; ++i) if (queues[i]) clReleaseCommandQueue(queues[i]);
    }
};

int main(int argc, char * argv[])
{
    std::string aocx_filename = "device.aocx";
    int platform_id = 1;
    int device_id = 0;
    int iterations = 4;
    int max_batch_size = 1024;
    int number_of_buffers = 2;

    argc--;
    argv++;

    int argi = 0;
    if (argc > argi) platform_id       = atoi(argv[argi++]);
    if (argc > argi) device_id         = atoi(argv[argi++]);
    if (argc > argi) iterations        = atoi(argv[argi++]);
    if (argc > argi) max_batch_size    = atoi(argv[argi++]);
    if (argc > argi) number_of_buffers = atoi(argv[argi++]);


    std::cout << "      platform_id: " << platform_id       << std::endl;
    std::cout << "        device_id: " << device_id         << std::endl;
    std::cout << "       iterations: " << iterations        << std::endl;
    std::cout << "   max_batch_size: " << max_batch_size    << std::endl;
    std::cout << "number_of_buffers: " << number_of_buffers << std::endl;

    // OpenCL init
    OCL ocl;
    ocl.init(aocx_filename, platform_id, device_id, true);

    FPipeGraph pipe(ocl, max_batch_size, number_of_buffers);
    pipe.start();

    // pushes to source

    std::thread t_source([&](){
        std::cout << "Source started!\n";
        size_t dataset_size = max_batch_size;
        std::vector< {{ source_data_type }} > dataset(dataset_size);
        fill_dataset(dataset, dataset_size);

        for (size_t i = 0; i < iterations; ++i) {
            std::cout << "Pushing " << iterations << " batch of size " << dataset_size << std::endl;
            pipe.push(dataset, dataset_size, i == (iterations - 1));
        }
    });

    std::thread t_sink([&](){
        std::cout << "Sink started!\n";

        size_t dataset_size = max_batch_size;

        bool shutdown = false;
        while (!shutdown) {
            cl_uint filled_size = 0;
            std::vector< {{ sink_data_type }} > data = pipe.pop(dataset_size, &filled_size, &shutdown);
            if (filled_size > 0) {
                if (!check_results(data, dataset_size)) {
                    std::cout << "ERROR in checking results!" << std::endl;
                }
                std::cout << "Received " << filled_size << " tuples (first_element = " << data[0].value << std::endl;
            }
            std::cout << "SHUTDOWN = " << shutdown << std::endl;
        }
    });

    t_source.join();
    t_sink.join();
    pipe.wait_and_stop();

    double elapsed_time = pipe.service_time();
    // Print results
    std::cout << COUT_HEADER << "Elapsed Time: " << COUT_FLOAT   << elapsed_time    << " s\n"
              << std::endl;

    pipe.clean();
    ocl.clean();

    return 0;
}
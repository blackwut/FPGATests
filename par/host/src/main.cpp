#include <iostream>

#include "options.hpp"
#include "benchmark.hpp"

int main(int argc, char * argv[])
{
    Options opt;
    opt.process_args(argc, argv);

    if (opt.k_base) {
        Benchmark b(K_TYPE::K_BASE, opt.k_base, opt.iterations, opt.size);
        b.start_and_wait(opt.platform, opt.device);
    }

    if (opt.k_unroll > 0) {
        Benchmark b(K_TYPE::K_UNROLL, opt.k_unroll, opt.iterations, opt.size);
        b.start_and_wait(opt.platform, opt.device);
    }

    if (opt.k_replica > 0) {
        Benchmark b(K_TYPE::K_REPLICA, opt.k_replica, opt.iterations, opt.size);
        b.start_and_wait(opt.platform, opt.device);
    }

    if (opt.k_ndrange > 0) {
        std::cout << "Not implemented yet!" << std::endl;
        // Benchmark b(K_TYPE::K_NDRANGE, opt.k_ndrange, opt.iterations, opt.size);
        // b.start_and_wait(opt.platform, opt.device);
    }

    if (opt.k_fuse > 0) {
        Benchmark b(K_TYPE::K_FUSE, opt.k_fuse, opt.iterations, opt.size);
        b.start_and_wait(opt.platform, opt.device);
    }

    return 0;
}
#include <iostream>

#include "options.hpp"
#include "benchmark.hpp"

int main(int argc, char * argv[])
{
    Options opt;
    opt.process_args(argc, argv);

    Benchmark b(1, opt.size);
    b.start_and_wait(opt.platform, opt.device);

    return 0;
}
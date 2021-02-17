#include <iostream>

#include "options.hpp"
#include "cpu.hpp"
#include "fpga.hpp"

int main(int argc, char * argv[])
{
    Options opt;
    opt.process_args(argc, argv);

    if (opt.cpu > 0) {
        CPU cpu;
        cpu.start_and_wait(opt.cpu, opt.size);
    }

    if (opt.fpga > 0) {
        FPGA fpga;
        fpga.start_and_wait(opt.fpga, opt.size, opt.platform, opt.device);
    }

    return 0;
}
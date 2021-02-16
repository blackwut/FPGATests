#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <getopt.h>

using namespace std;

struct Options
{
    int size;
    int cpu;
    int fpga;
    int platform;
    int device;

    Options()
    : size(1024)
    , cpu(0)
    , fpga(0)
    , platform(0)
    , device(0)
    {}

    void print_help()
    {
        cout << "\t-n  --size                Set the number of items               \n"
             << "\t-c  --cpu  N (1, 4, 8)    Perform benchmark on CPU with FastFlow\n"
             << "\t-f  --fpga N (1, 4, 8)    Perform benchmark on FPGA with OpenCL \n"
             << "\t-p  --platform id         Specify the OpenCL platform index     \n"
             << "\t-d  --device id           Specify the OpenCL device index       \n"
             << endl;
        exit(1);
    }

    void process_args(int argc, char * argv[])
    {
        opterr = 0;

        const char * const short_opts = "n:c:f:p:d:";
        const option long_opts[] = {
            {"size",     optional_argument, nullptr, 'n'},
            {"cpu",      optional_argument, nullptr, 'c'},
            {"fpga",     optional_argument, nullptr, 'f'},
            {"platform", optional_argument, nullptr, 'p'},
            {"device",   optional_argument, nullptr, 'd'},
            {"help",     no_argument,       nullptr, 'h'},
            {nullptr,    no_argument,       nullptr,   0}
        };

        int int_opt = -1;

        while (1) {
            const int opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

            if (opt < 0) break;

            switch (opt) {
                case 'n':
                    if ((int_opt = stoi(optarg)) < 0) {
                        cerr << "Please enter a valid number of items per iteration" << endl;
                        exit(1);
                    }
                    size = int_opt;
                    break;
                case 'c':
                    if ((int_opt = stoi(optarg)) <= 0) {
                        cerr << "Please enter a valid number of workers" << endl;
                        exit(1);
                    }
                    cpu = int_opt;
                    break;
                case 'f':
                    if ((int_opt = stoi(optarg)) <= 0) {
                        cerr << "Please enter a valid number of workers" << endl;
                        exit(1);
                    }
                    fpga = int_opt;
                    break;
                case 'p':
                    if ((int_opt = stoi(optarg)) < 0) {
                        cerr << "Please enter a valid platform" << endl;
                        exit(1);
                    }
                    platform = int_opt;
                    break;
                case 'd':
                    if ((int_opt = stoi(optarg)) < 0) {
                        cerr << "Please enter a valid device" << endl;
                        exit(1);
                    }
                    device = int_opt;
                    break;

                case 'h':
                case '?':
                default:
                    print_help();
                    break;
            }
        }

        if (cpu != 0 and cpu != 1 and cpu != 4 and cpu != 8) {
            cerr << "Please use 1, 4, or 8 as --cpu argument!\n";
            exit(-1);
        }

        if (fpga != 0 and fpga != 1 and fpga != 4 and fpga != 8) {
            cerr << "Please use 1, 4, or 8 as --fpga argument!\n";
            exit(-1);
        }
    }
};
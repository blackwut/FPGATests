#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <getopt.h>

using namespace std;

struct Options
{
    int platform;
    int device;
    int iterations;
    int size;

    Options()
    : platform(0)
    , device(0)
    , iterations(4)
    , size(1024)
    {}

    void print_help()
    {
        cout << "\t-p  --platform id     Specify the OpenCL platform index     \n"
                "\t-d  --device id       Specify the OpenCL device index       \n"
                "\t-i  --iterations      Set the number of iterations          \n"
                "\t-n  --size            Set the number of items per iteration \n";
        exit(1);
    }

    void process_args(int argc, char * argv[])
    {
        opterr = 0;

        const char * const short_opts = "p:d:i:n:";
        const option long_opts[] = {
                {"platform",    optional_argument, nullptr, 'p'},
                {"device",      optional_argument, nullptr, 'd'},
                {"iterations",  optional_argument, nullptr, 'i'},
                {"size",        optional_argument, nullptr, 'n'},
                {"help",        no_argument,       nullptr, 'h'},
                {nullptr,       no_argument,       nullptr,   0}
        };

        int int_opt = -1;

        while (1) {
            const int opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

            if (opt < 0) break;

            switch (opt) {
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
                case 'i':
                    if ((int_opt = stoi(optarg)) < 0) {
                        cerr << "Please enter a valid number of iterations" << endl;
                        exit(1);
                    }
                    iterations = int_opt;
                    break;
                case 'n':
                    if ((int_opt = stoi(optarg)) < 0) {
                        cerr << "Please enter a valid number of items per iteration" << endl;
                        exit(1);
                    }
                    size = int_opt;
                    break;

                case 'h':
                case '?':
                default:
                    print_help();
                    break;
            }
        }
    }
};
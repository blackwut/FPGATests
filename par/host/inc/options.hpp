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
    bool k_base;
    int k_unroll;
    int k_replica;
    int k_ndrange;
    int k_fuse;
    bool no_global;

    Options()
    : platform(0)
    , device(0)
    , iterations(4)
    , size(1024)
    , k_base(false)
    , k_unroll(-1)
    , k_replica(-1)
    , k_ndrange(-1)
    , k_fuse(-1)
    , no_global(false)
    {}

    void print_help()
    {
        cout << "\t-p  --platform id     Specify the OpenCL platform index     \n"
                "\t-d  --device id       Specify the OpenCL device index       \n"
                "\t-i  --iterations      Set the number of iterations          \n"
                "\t-n  --size            Set the number of items per iteration \n"
                "\t-B  --base            Benchmark of kernel type `base`       \n"
                "\t-U  --unroll N        Benchmark of kernel type `unroll`     \n"
                "\t-R  --replica N       Benchmark of kernel type `replica`    \n"
                "\t-N  --ndrange N       Benchmark of kernel type `ndrange`    \n"
                "\t-F  --fuse N          Benchmark of kernel type `fuse`       \n"
                "\t-G  --no_global       Tuples are generated directly by FPGA \n";
        exit(1);
    }

    void process_args(int argc, char * argv[])
    {
        opterr = 0;

        const char * const short_opts = "p:d:i:n:BU:R:N:F:G";
        const option long_opts[] = {
                {"platform",    optional_argument, nullptr, 'p'},
                {"device",      optional_argument, nullptr, 'd'},
                {"iterations",  optional_argument, nullptr, 'i'},
                {"size",        optional_argument, nullptr, 'n'},
                {"base",        optional_argument, nullptr, 'B'},
                {"unroll",      optional_argument, nullptr, 'U'},
                {"replica",     optional_argument, nullptr, 'R'},
                {"ndrange",     optional_argument, nullptr, 'N'},
                {"fuse",        optional_argument, nullptr, 'F'},
                {"no_global",   optional_argument, nullptr, 'G'},
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
                case 'B':
                    k_base = true;
                    break;
                case 'U':
                    int_opt = stoi(optarg);
                    if (!(int_opt == 2 or int_opt == 4 or int_opt == 8)) {
                        cerr << "Please enter a valid number of CUs (2, 4, 8)." << endl;
                        exit(1);
                    }
                    k_unroll = int_opt;
                    break;
                case 'R':
                    int_opt = stoi(optarg);
                    if (!(int_opt == 2 or int_opt == 4 or int_opt == 8)) {
                        cerr << "Please enter a valid number of CUs (2, 4, 8)." << endl;
                        exit(1);
                    }
                    k_replica = int_opt;
                    break;
                case 'N':
                    int_opt = stoi(optarg);
                    if (!(int_opt == 2 or int_opt == 4 or int_opt == 8)) {
                        cerr << "Please enter a valid number of CUs (2, 4, 8)." << endl;
                        exit(1);
                    }
                    k_ndrange = int_opt;
                    break;
                case 'F':
                    int_opt = stoi(optarg);
                    if (!(int_opt == 2 or int_opt == 4 or int_opt == 8)) {
                        cerr << "Please enter a valid number of CUs (2, 4, 8)." << endl;
                        exit(1);
                    }
                    k_fuse = int_opt;
                    break;
                case 'G':
                    no_global = true;
                    break;

                case 'h':
                case '?':
                default:
                    print_help();
                    break;
            }
        }

        if (!k_base and !k_unroll and !k_replica and !k_ndrange and !k_fuse) {
            cerr << "Please specify at least one kernel type: `--base`, `--unroll`, `--replica`, `--ndrange`, `--fuse`\n";
            exit(0);
        }
    }
};
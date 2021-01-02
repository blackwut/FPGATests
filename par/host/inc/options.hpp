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
    bool k_unroll;
    bool k_replica;
    bool k_replica_new;
    bool k_ndrange;

    Options()
    : platform(0)
    , device(0)
    , iterations(32)
    , size(1024)
    , k_base(false)
    , k_unroll(false)
    , k_replica(false)
    , k_replica_new(false)
    , k_ndrange(false)
    {}

    void print_help()
    {
        cout << "\t-p  --platform        Specify the OpenCL platform index     \n"
                "\t-d  --device          Specify the OpenCL device index       \n"
                "\t-i  --iterations      Set the number of iterations          \n"
                "\t-n  --size            Set the number of items per iteration \n"
                "\t-B  --base            Benchmark of kernel type `base`       \n"
                "\t-U  --unroll          Benchmark of kernel type `unroll`     \n"
                "\t-R  --replica         Benchmark of kernel type `replica`    \n"
                "\t-C  --replica_new     Benchmark of kernel type `replica_new`\n"
                "\t-N  --ndrange         Benchmark of kernel type `NDRange     \n";
        exit(1);
    }

    void process_args(int argc, char * argv[])
    {
        opterr = 0;

        const char * const short_opts = "p:d:i:n:BURCN";
        const option long_opts[] = {
                {"platform",    optional_argument, nullptr, 'p'},
                {"device",      optional_argument, nullptr, 'd'},
                {"iterations",  optional_argument, nullptr, 'i'},
                {"size",        optional_argument, nullptr, 'n'},
                {"base",        optional_argument, nullptr, 'B'},
                {"unroll",      optional_argument, nullptr, 'U'},
                {"replica",     optional_argument, nullptr, 'R'},
                {"replica_new", optional_argument, nullptr, 'C'},
                {"ndrange",     optional_argument, nullptr, 'N'},
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
                    k_unroll = true;
                    break;
                case 'R':
                    k_replica = true;
                    break;
                case 'C':
                    k_replica_new = true;
                    break;
                case 'N':
                    k_ndrange = true;
                    break;

                case 'h':
                case '?':
                default:
                    print_help();
                    break;
            }
        }

        if (!k_base and !k_unroll and !k_replica and !k_replica_new and !k_ndrange) {
            cerr << "Please specify at least one kernel type: `--base`, `--unroll`, `--replica`, `--replica_new`, `--ndrange`\n";
            exit(0);
        }
    }
};
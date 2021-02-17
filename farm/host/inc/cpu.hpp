#include <ff/ff.hpp>

#include "common.hpp"
#include "utils.hpp"

using namespace ff;
using namespace std;


struct Emitter: ff_monode_t<int>
{
    size_t workers;
    size_t N;

    Emitter(const size_t workers, const size_t N)
    : workers(workers)
    , N(N)
    {}
    
    int * svc(int *) {
        for(size_t i = 1; i <= N; ++i) {
            ff_send_out_to((int *)i, i % workers);
        }
        return EOS;
    }
};

struct Worker: ff_node_t<int>
{
    int * svc(int * in) {
        return ++in;
    }
};

struct Collector: ff_minode_t<int>
{
    int N;

    Collector(const int N)
    : N(N)
    {}
    
    int * svc(int *) {
        --N;
        return GO_ON;
    }

    void svc_end() {
        if (N != 0) {
            cerr << "ERROR in CPU test (Collector)\n" << endl;
            abort();
        }
    }
};

struct CPU
{
    void start_and_wait(const size_t workers, const size_t size)
    {
        cout << "CPU Test started with " << workers << " workers!" << endl;

        size_t ec_size = round_up(size, workers);

        Emitter E(workers, ec_size);
        Collector C(ec_size);
        vector< unique_ptr<ff_node> > W;
        
        for(size_t i = 0; i < workers;++i) {
            W.push_back(make_unique<Worker>());
        }

        ff_Farm<> farm(move(W), E, C);
        if (farm.run_and_wait_end() < 0) {
            cerr << "ERROR in CPU test (Farm)\n" << endl;
            abort();
        }

        double elapsed_time = farm.ffTime() / 1.0e3;
        double throughput = ec_size / elapsed_time;

        cout << COUT_HEADER << "Items: "        << COUT_INTEGER << ec_size          << "\n"
             << COUT_HEADER << "Elapsed Time: " << COUT_FLOAT   << elapsed_time     << " s\n"
             << COUT_HEADER << "Throughput: "   << COUT_INTEGER << (long)throughput << " incr/s\n"
             << endl;
    }
};

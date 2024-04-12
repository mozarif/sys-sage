#include <iostream>
#include <chrono>
#include <unistd.h>

#include "sys-sage.hpp"

#include "defines.hpp"


//PARAMS TO SET
#define TIMER_WARMUP 32
#define TIMER_REPEATS 128

#define REPEATS_START_SKIP 4
#define REPEATS 64

using namespace std;

uint64_t get_timer_overhead(int repeats, int warmup);
int main(int argc, char *argv[])
{
    std::string path_prefix(argv[0]);
    std::size_t found = path_prefix.find_last_of("/\\");
    path_prefix=path_prefix.substr(0,found) + "/";
    string topoPath = path_prefix + "example_data/skylake_hwloc.xml";
    string gpuPath = path_prefix + "example_data/ampere_gpu_topo.csv";


    std::chrono::high_resolution_clock::time_point t_start, t_end;
    uint64_t timer_overhead = get_timer_overhead(TIMER_REPEATS, TIMER_WARMUP);
    cout << "timer overhead, " << timer_overhead << endl;

    Node* n = new Node(0);
    int ret = parseHwlocOutput(n, topoPath);

    //frequency
    for(int i = 0; i<REPEATS+REPEATS_START_SKIP; i++)
    {
        if(i>=REPEATS_START_SKIP)
            cout << "freq, " << i << ", ";
        t_start = std::chrono::high_resolution_clock::now();
        n->RefreshCpuCoreFrequency(false);
        t_end = std::chrono::high_resolution_clock::now();
        if(i>=REPEATS_START_SKIP)
             cout << "outside, " << t_end.time_since_epoch().count()-t_start.time_since_epoch().count() << endl;
    }

#ifdef CAT_AWARE
    //L3 partitioning
    for(int i = 0; i<REPEATS+REPEATS_START_SKIP; i++)
    {
        if(i>=REPEATS_START_SKIP)
            cout << "cat, " << i << ", ";
        t_start = std::chrono::high_resolution_clock::now();
        n->UpdateL3Partitioning();
        t_end = std::chrono::high_resolution_clock::now();
        usleep(100000);
        usleep(100000);
        if(i>=REPEATS_START_SKIP)
             cout << "outside, " << t_end.time_since_epoch().count()-t_start.time_since_epoch().count() << endl;
    }
#endif

#ifdef NVIDIA_MIG
    //nvidia mig
    int ret2 = parseMt4gTopo(n, gpuPath, 999, ";");
    Chip* gpu = (Chip*)(n->GetChild(999));
    for(int i = 0; i<REPEATS+REPEATS_START_SKIP; i++)
    {
        if(i>=REPEATS_START_SKIP)
            cout << "mig, " << i << ", ";
        t_start = std::chrono::high_resolution_clock::now();
        gpu->UpdateMIGSettings();
        t_end = std::chrono::high_resolution_clock::now();
        if(i>=REPEATS_START_SKIP)
             cout << "outside, " << t_end.time_since_epoch().count()-t_start.time_since_epoch().count() << endl;
    }
#endif

    return 0;
}

uint64_t get_timer_overhead(int repeats, int warmup)
{
    std::chrono::high_resolution_clock::time_point t_start, t_end;
    uint64_t time = 0;
    int tmp_sum=0; //to make sure it does not get rearranged?
    //uint64_t time_arr[10];
    for(int i=0; i<repeats+warmup; i++)
    {
        t_start = std::chrono::high_resolution_clock::now();
        tmp_sum+=i;
        t_end = std::chrono::high_resolution_clock::now();
        if(i>=warmup)
            time += t_end.time_since_epoch().count()-t_start.time_since_epoch().count();
    }
    if(tmp_sum > 0)
        time = time/repeats;
    return time;
}
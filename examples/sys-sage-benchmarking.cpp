

#include <cstdint>
#include <iostream>
//#include <hwloc.h>
#include <chrono>
#include <sys/types.h>

#include "sys-sage.hpp"

////////////////////////////////////////////////////////////////////////
//PARAMS TO SET
#define TIMER_WARMUP 32
#define TIMER_REPEATS 128

////////////////////////////////////////////////////////////////////////
using namespace std::chrono;


//int hwloc_dump_xml(const char *filename);
uint64_t get_timer_overhead(int repeats, int warmup);


//this file benchmarks and prints out performance information about basic operations with sys-sage
int main(int argc, char *argv[])
{
    std::string path_prefix(argv[0]);
    std::size_t found = path_prefix.find_last_of("/\\");
    path_prefix=path_prefix.substr(0,found) + "/";
    string xmlPath = path_prefix + "example_data/skylake_hwloc.xml";
    string bwPath = path_prefix + "example_data/skylake_caps_numa_benchmark.csv";
    string mt4gPath = path_prefix + "example_data/ampere_gpu_topo.csv";

    high_resolution_clock::time_point t_start, t_end;
    uint64_t timer_overhead = get_timer_overhead(TIMER_REPEATS, TIMER_WARMUP);

    Topology* t = new Topology();
    Node* n = new Node(t, 1);
    vector<Component*> hwlocComponentList, mt4gComponentList, allComponentList;

    uint64_t time_createNewComponent = UINT64_MAX;

    for (int i = 0; i < 1000; i++) {
        n->Delete(false);
        t_start = high_resolution_clock::now();
        n = new Node(t, 1);
        t_end = high_resolution_clock::now();
        uint64_t time = t_end.time_since_epoch().count() - t_start.time_since_epoch().count() - timer_overhead;
        if (time < time_createNewComponent) {
            time_createNewComponent = time;
        }
    }

    //time hwloc_parser

    uint64_t time_parseHwlocOutput = UINT64_MAX;
    int ret = parseHwlocOutput(n, xmlPath);
    for (int i = 0; i < 1000; i++) {
        t_start = high_resolution_clock::now();
        ret = parseHwlocOutput(n, xmlPath);
        t_end = high_resolution_clock::now();
        uint64_t time = t_end.time_since_epoch().count() - t_start.time_since_epoch().count() - timer_overhead;
        if (time < time_parseHwlocOutput) {
            time_parseHwlocOutput = time;
        }
        if (ret != 0) {
            cout << "failed parsing hwloc" << endl;
            return 1;
        }

    }

    //time get a vector with all Components (of hwloc parser)
    uint64_t time_GetHwlocSubtreeNodeList = UINT64_MAX;
    for (int i = 0; i < 1000; i++) {
        hwlocComponentList.clear();
        t_start = high_resolution_clock::now();
        n->GetComponentsInSubtree(&hwlocComponentList);
        t_end = high_resolution_clock::now();
        uint64_t time = t_end.time_since_epoch().count() - t_start.time_since_epoch().count() - timer_overhead;
        if (time < time_GetHwlocSubtreeNodeList) {
            time_GetHwlocSubtreeNodeList = time;
        }
    }

    //time caps-numa-benchmark parser
    uint64_t time_parseCapsNumaBenchmark = UINT64_MAX;
    for (int i = 0; i < 1000; i++) {
        t_start = high_resolution_clock::now();
        ret = parseCapsNumaBenchmark((Component*)n, bwPath, ";");
        t_end = high_resolution_clock::now();
        uint64_t time = t_end.time_since_epoch().count() - t_start.time_since_epoch().count() - timer_overhead;
        if (time < time_parseCapsNumaBenchmark) {
            time_parseCapsNumaBenchmark = time;
        }
        if (ret != 0) {
            cout << "failed parsing caps-numa-benchmark" << endl;
            return 1;
        }
    }

    //get num caps-benchmark DataPaths
    int caps_dataPaths = 0;
    vector<DataPath*>* capsDataPaths;
    for (Component* gpu_c : hwlocComponentList) {
        capsDataPaths = gpu_c->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
        caps_dataPaths += capsDataPaths->size();
    }

    //get size of hwloc representation + caps-benchmark DataPaths
    unsigned hwloc_component_size = 0;
    unsigned caps_numa_dataPathSize = 0;
    unsigned total_size = n->GetTopologySize(&hwloc_component_size, &caps_numa_dataPathSize);
    cout << "hwloc_component_size, " << hwloc_component_size << ", caps_numa_dataPathSize, " << caps_numa_dataPathSize << ", total_size, " << total_size << endl;

    //for NUMA 0 get NUMA with min BW
    uint64_t time_getNumaMaxBw = UINT64_MAX;

    Numa* numa = (Numa*)n->GetSubcomponentById(0, SYS_SAGE_COMPONENT_NUMA);
    if (numa == NULL) { cerr << "numa 0 not found in sys-sage" << endl; return 1; }
    unsigned int max_bw = 0;
    Component* max_bw_component = NULL;
    for(int i = 0; i < 1000; i++) {
        t_start = high_resolution_clock::now();
        vector<DataPath*>* dp = numa->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
        for (auto it = std::begin(*dp); it != std::end(*dp); ++it) {
            if ((*it)->GetBandwidth() > max_bw) {
                max_bw = (*it)->GetBandwidth();
                max_bw_component = (*it)->GetTarget();
            }
        }
        cout << "Max bw is: " << max_nw << endl;
        t_end = high_resolution_clock::now();
        uint64_t time = t_end.time_since_epoch().count() - t_start.time_since_epoch().count() - timer_overhead;
        if(time < time_getNumaMaxBw) {
            time_getNumaMaxBw = time;
        }
    }
    

    //time mt4g parser
    Chip* gpu = new Chip(n, 100, "GPU");
    uint64_t time_parseMt4g = UINT64_MAX;

    for(int i = 0; i < 100; i++) {
        i = 99;
        gpu->Delete();
        t_start = high_resolution_clock::now();
        Chip* gpu = new Chip(n, 100, "GPU");
        ret = parseMt4gTopo(gpu, mt4gPath, ";");
        t_end = high_resolution_clock::now();
        uint64_t time = t_end.time_since_epoch().count() - t_start.time_since_epoch().count() - timer_overhead;
        if (time < time_parseMt4g) {
            time_parseMt4g = time;
        }
        if (ret != 0) {
            cout << "failed parsing mt4g" << endl;
            return 1;
        }
    }

    //time get a vector with all Components (of mt4g parser)
    uint64_t time_GetMt4gSubtreeNodeList = UINT64_MAX;

    for (int i = 0; i < 100; i++) {
        mt4gComponentList.clear();
        t_start = high_resolution_clock::now();
        gpu->GetComponentsInSubtree(&mt4gComponentList);
        t_end = high_resolution_clock::now();
        uint64_t time = t_end.time_since_epoch().count() - t_start.time_since_epoch().count() - timer_overhead;
        if (time < time_GetMt4gSubtreeNodeList) {
            time_GetMt4gSubtreeNodeList = time;
        }
    }
    
    //time get a vector with all Components
    uint64_t time_GetAllComponentsList = UINT64_MAX;

    for (int i = 0; i < 1000; i++) {
        allComponentList.clear();
        t_start = high_resolution_clock::now();
        t->GetComponentsInSubtree(&allComponentList);
        t_end = high_resolution_clock::now();
        uint64_t time = t_end.time_since_epoch().count() - t_start.time_since_epoch().count() - timer_overhead;
        if (time < time_GetAllComponentsList) {
            time_GetAllComponentsList = time;
        }
    }
    //get num mt4g DataPaths
    int mt4g_dataPaths = 0;
    vector<DataPath*>* componentDataPaths;
    for (Component* gpu_c : mt4gComponentList) {
        componentDataPaths = gpu_c->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
        mt4g_dataPaths += componentDataPaths->size();
    }

    /////////////////print results
    cout << "time_parseHwlocOutput, " << time_parseHwlocOutput << endl;
    cout << ", time_parseCapsNumaBenchmark, " << time_parseCapsNumaBenchmark << endl;
    cout << ", time_parseMt4g, " << time_parseMt4g << endl;
    cout << ", time_GetHwlocSubtreeNodeList, " << time_GetHwlocSubtreeNodeList << endl;
    cout << ", time_GetMt4gSubtreeNodeList, " << time_GetMt4gSubtreeNodeList << endl;
    cout << ", time_GetAllComponentsList, " << time_GetAllComponentsList << endl;

    cout << ", hwloc_components, " << hwlocComponentList.size() << endl;
    cout << ", caps_dataPaths, " << caps_dataPaths << endl;
    cout << ", mt4g_components, " << mt4gComponentList.size() << endl;
    cout << ", mt4g_dataPaths, "<< mt4g_dataPaths << endl;
    cout << ", all_components, " << allComponentList.size() << endl;

    cout << ", time_getNumaMaxBw, " << time_getNumaMaxBw << endl;
    if(false)
        cout << "; bw; " << max_bw << "; ComponentId; " << max_bw_component->GetId() << endl;
    cout << ", time_createNewComponent, " << time_createNewComponent << endl;

    cout << ", hwloc_component_size[B], " << hwloc_component_size << endl;
    cout << ", caps_numa_dataPathSize[B], " << caps_numa_dataPathSize << endl;
    cout << ", total_size, " << total_size << endl;

    cout << endl;

    return 0;
}

uint64_t get_timer_overhead(int repeats, int warmup)
{
    high_resolution_clock::time_point t_start, t_end;
    uint64_t time = 0;
    //uint64_t time_arr[10];
    for(int i=0; i<repeats+warmup; i++)
    {
        t_start = high_resolution_clock::now();
        t_end = high_resolution_clock::now();
        if(i>=warmup)
            time += t_end.time_since_epoch().count()-t_start.time_since_epoch().count();
    }
    time = time/repeats;
    return time;
}

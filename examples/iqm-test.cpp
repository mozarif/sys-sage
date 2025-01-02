#include <iomanip>
#include "sys-sage.hpp"
#include <chrono>
#include <queue>
#include <algorithm>

// #define PERF_MEASUREMENTS
///////////////////// FoMaC functionality /////////////////////
////// This is a use-case-specific functionality, which would be a part of the sys-sage FoMaC, not the core sys-sage. hence, it is here as a separate code
#define T_WEIGHT            0.1
#define Q1_FID_WEIGHT       0.2
#define READOUT_FID_WEIGHT  0.4
#define TWO_Q_FID_WEIFHT    0.3

void calculateQubitWeight(Qubit* q, int tsForHistory = -1, double T1_max = 1, double  T2_max = 1, double  q1_fidelity_max = 1, double readout_fidelity_max = 1, double two_q_fidelity_max = 1 )
{
    if(!q->attrib.contains("qubit_weight"))
        q->attrib["qubit_weight"] = new double();

    double min_t = q->GetT1() > q->GetT2()? q->GetT2(): q->GetT1();
    double max_t_max = T1_max > T2_max ? T1_max : T2_max;

    double q_weight = min_t/max_t_max + q->Get1QFidelity()/q1_fidelity_max + q->GetReadoutFidelity()/readout_fidelity_max ;

    double coupling_map_fidelity = 0.0;
    int num_neighbours = 0;
    for(Relation* r : q->relations)
    {
        if(r->GetRelationType() == SYS_SAGE_RELATION_COUPLING_MAP)
        {
            coupling_map_fidelity += (static_cast<CouplingMap*>(r))->GetFidelity();
            num_neighbours++;
        }
    }
    q_weight += (coupling_map_fidelity/two_q_fidelity_max)/num_neighbours;

    *static_cast<double*>(q->attrib["qubit_weight"]) = q_weight;
    if(tsForHistory > 0)
    {
        //check if weight_history exists; if not, create it -- vector of tuples <timestamp,weight>
        if(! q->attrib.contains("weight_history"))
            q->attrib["weight_history"] = reinterpret_cast<void*>(new std::vector<std::tuple<int,double>>());
        auto rh = reinterpret_cast<std::vector<std::tuple<int,double>>*>(q->attrib["weight_history"]);
        rh->emplace_back(tsForHistory, q_weight);
    }

}

int calculateAllWeights(QuantumBackend* backend, int tsForHistory = -1)
{
    double T1_max = *static_cast<double*>(backend->attrib["T1_max"]);
    double T2_max = *static_cast<double*>(backend->attrib["T2_max"]);
    double q1_fidelity_max = *static_cast<double*>(backend->attrib["q1_fidelity_max"]);
    double readout_fidelity_max = *static_cast<double*>(backend->attrib["readout_fidelity_max"]);
    double two_q_fidelity_max = *static_cast<double*>(backend->attrib["two_q_fidelity_max"]);
    for(Component* c : *backend->GetChildren())
    {
        if(c->GetComponentType() == SYS_SAGE_COMPONENT_QUBIT)
        {
            Qubit* q = static_cast<Qubit*>(c);
            calculateQubitWeight(q, tsForHistory, T1_max, T2_max, q1_fidelity_max, readout_fidelity_max, two_q_fidelity_max );
        }
    }
    return 0;
}

vector<Qubit*> findTopNQubits(QuantumBackend* backend, int n)
{
    auto compare = [](Qubit* a, Qubit* b) { return *static_cast<double*>(a->attrib["qubit_weight"]) > *static_cast<double*>(b->attrib["qubit_weight"]) ;};
    std::priority_queue<Qubit*, std::vector<Qubit*>, decltype(compare)> min_heap(compare);

    for(Component* c : *backend->GetChildren())
    {
        Qubit* q = static_cast<Qubit*>(c);
        
        double qw = *static_cast<double*>(q->attrib["qubit_weight"]);
        min_heap.push(q);
        if (min_heap.size() > n) {
            min_heap.pop(); // Remove the smallest weight to keep only the top n
        }
    }

    std::vector<Qubit*> result;
    while (!min_heap.empty()) {
        result.push_back(min_heap.top());
        min_heap.pop();
    }
    std::reverse(result.begin(), result.end());
    return result;
}

///////////////////////////////////////////////////////////////

int getTs(std::string filename)
{
    size_t lastUnderscore = filename.find_last_of('_');
    size_t lastDot = filename.find_last_of('.');
    std::string result = (lastUnderscore != std::string::npos && lastDot != std::string::npos && lastUnderscore < lastDot) 
                         ? filename.substr(lastUnderscore + 1, lastDot - lastUnderscore - 1) 
                         : "";
    return std::stoi(result);
}


int main(int argc, char *argv[])
{
    std::cout << std::setprecision(15);
    QuantumBackend* b = new QuantumBackend();  
    // string IQMPathPrefix = "/Users/stepan/phd/repos/q-sys-sage/tmp-qc-data/database/";
    string IQMPath;

    string bwPath;
    if(argc < 2){
        std::string path_prefix(argv[0]);
        std::size_t found = path_prefix.find_last_of("/\\");
        path_prefix=path_prefix.substr(0,found) + "/";
        IQMPath = path_prefix + "example_data/test_data_iqm_16.json";
    }
    else if(argc == 2){
        IQMPath = argv[1];
    }
    else{
        std::cerr << "Wrong input params" << std::endl;
        return 1;
    }

#ifndef PERF_MEASUREMENTS
    std::cout << "-- Parsing IQM output from file " << IQMPath << std::endl;
#else
    std::cout << "num_qubits;" << num_qubits;
#endif
    if(parseIQM(b, IQMPath, 0, -1, true) != 0) { //adds topo to a next node
        return 1;
    }
    
    // int IQMPathTs;
    // IQMPath = IQMPathPrefix + "calibration_data_Q-Exa_20241013.json";
    // IQMPathTs =  getTs(IQMPath);
    // std::cout << "-- Parsing IQM output from file " << IQMPath << ", ts " << IQMPathTs << std::endl;
    // if(parseIQM(b, IQMPath, 0, IQMPathTs) != 0) { //adds topo to a next node
    //     return 1;
    // }
    // calculateAllWeights(b,IQMPathTs);

    // //calibration_data_Q-Exa_20241013.json already included in the first step
    // //string IQMFileNames[] = {"calibration_data_Q-Exa_20241014.json","calibration_data_Q-Exa_20241015.json","calibration_data_Q-Exa_20241016.json","calibration_data_Q-Exa_20241017.json"};
    // string IQMFileNames[] = {"calibration_data_Q-Exa_20241014.json","calibration_data_Q-Exa_20241015.json","calibration_data_Q-Exa_20241016.json","calibration_data_Q-Exa_20241017.json","calibration_data_Q-Exa_20241018.json","calibration_data_Q-Exa_20241019.json","calibration_data_Q-Exa_20241020.json","calibration_data_Q-Exa_20241021.json","calibration_data_Q-Exa_20241022.json","calibration_data_Q-Exa_20241023.json","calibration_data_Q-Exa_20241024.json","calibration_data_Q-Exa_20241025.json","calibration_data_Q-Exa_20241026.json","calibration_data_Q-Exa_20241027.json","calibration_data_Q-Exa_20241028.json","calibration_data_Q-Exa_20241029.json","calibration_data_Q-Exa_20241030.json","calibration_data_Q-Exa_20241031.json","calibration_data_Q-Exa_20241101.json","calibration_data_Q-Exa_20241102.json","calibration_data_Q-Exa_20241103.json","calibration_data_Q-Exa_20241104.json","calibration_data_Q-Exa_20241105.json","calibration_data_Q-Exa_20241106.json","calibration_data_Q-Exa_20241107.json","calibration_data_Q-Exa_20241108.json","calibration_data_Q-Exa_20241109.json","calibration_data_Q-Exa_20241110.json","calibration_data_Q-Exa_20241111.json","calibration_data_Q-Exa_20241112.json","calibration_data_Q-Exa_20241113.json","calibration_data_Q-Exa_20241114.json","calibration_data_Q-Exa_20241115.json","calibration_data_Q-Exa_20241116.json","calibration_data_Q-Exa_20241117.json","calibration_data_Q-Exa_20241118.json","calibration_data_Q-Exa_20241119.json","calibration_data_Q-Exa_20241120.json","calibration_data_Q-Exa_20241121.json","calibration_data_Q-Exa_20241122.json","calibration_data_Q-Exa_20241123.json","calibration_data_Q-Exa_20241124.json","calibration_data_Q-Exa_20241125.json","calibration_data_Q-Exa_20241126.json","calibration_data_Q-Exa_20241127.json","calibration_data_Q-Exa_20241128.json","calibration_data_Q-Exa_20241129.json","calibration_data_Q-Exa_20241130.json","calibration_data_Q-Exa_20241201.json","calibration_data_Q-Exa_20241202.json","calibration_data_Q-Exa_20241203.json","calibration_data_Q-Exa_20241204.json","calibration_data_Q-Exa_20241205.json"};

    // for(string file : IQMFileNames)
    // {
    //     IQMPath = IQMPathPrefix + file;
    //     IQMPathTs =  getTs(IQMPath);
    //     std::cout << "-- Parsing IQM output from file " << IQMPath << ", ts " << IQMPathTs << std::endl;
    //     if(parseIQM(b, IQMPath, 0, IQMPathTs, false) != 0) { //adds topo to a next node
    //         return 1;
    //     }
    //     calculateAllWeights(b,IQMPathTs);
    // }
    // cout << "-- End parseIQM" << endl;


#ifdef PERF_MEASUREMENTS
    auto start = std::chrono::high_resolution_clock::now();
#endif
    calculateAllWeights(b);
#ifdef PERF_MEASUREMENTS
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<long long, std::nano> duration4 = end - start;
#endif

#ifdef PERF_MEASUREMENTS
    start = std::chrono::high_resolution_clock::now();
#endif
    vector<Qubit*> res = findTopNQubits(b, 10);
#ifdef PERF_MEASUREMENTS
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<long long, std::nano> duration5 = end - start;

    std::cout << ";" << duration4.count() << ";" << duration5.count() << std::endl;
#endif

    // cout << "---------------- Printing the configuration of IQM Backend----------------" << endl;
    // b->PrintSubtree();
    
    // for(Component * c : *b->GetChildren() )
    // {
    //     if(c->GetComponentType() == SYS_SAGE_COMPONENT_QUBIT)
    //     {
    //         Qubit* q = static_cast<Qubit*>(c);
    //         std::cout << "Qubit " << q->GetId() << " has coupling map { ";
    //         for(Relation* r : q->relations)
    //         {
    //             if(r->GetRelationType() == SYS_SAGE_RELATION_COUPLING_MAP)
    //             {
    //                 CouplingMap* cm = static_cast<CouplingMap*>(r); 
    //                 std::cout << (cm->components[0] == q ? cm->components[1]->GetId() : cm->components[0]->GetId() )<< " ";
    //             }      
    //         }
    //         std::cout << "} and weight = " << *static_cast<double*>(q->attrib["qubit_weight"]) << "\n";
    //     }
    // }
    

    std::ofstream file("output.csv");
    if (!file.is_open()) 
    {
        std::cerr << "Failed to open file for writing." << std::endl;
        return 1;
    }

    file << std::fixed << std::setprecision(14);
    for(Component * c : *b->GetChildren() )
    {
        if(c->attrib.contains("weight_history"))
        {
            auto rh = reinterpret_cast<std::vector<std::tuple<int,double>>*>(c->attrib["weight_history"]);
            for(auto entry: *rh)
                file << std::get<0>(entry) << "," << c->GetId() << "," << c->relations.size() << "," << std::get<1>(entry) << "\n";
        }
    }
    
    return 0;
}

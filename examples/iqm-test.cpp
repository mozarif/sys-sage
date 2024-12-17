#include <iomanip>
#include "sys-sage.hpp"

///////////////////// FoMaC functionality /////////////////////
////// This is a use-case-specific functionality, which would be a part of the sys-sage FoMaC, not the core sys-sage. hence, it is here as a separate code
void calculateQubitWeight(Qubit* q, int tsForHistory = -1)
{
    if(!q->attrib.contains("qubit_weight"))
        q->attrib["qubit_weight"] = new double();

    double qw = q->GetT1() + q->GetT2() + q->Get1QFidelity() + q->GetReadoutFidelity() ;

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
    qw += coupling_map_fidelity/num_neighbours;

    *static_cast<double*>(q->attrib["qubit_weight"]) = qw;
    if(tsForHistory > 0)
    {
        //check if weight_history exists; if not, create it -- vector of tuples <timestamp,weight>
        if(! q->attrib.contains("weight_history"))
            q->attrib["weight_history"] = reinterpret_cast<void*>(new std::vector<std::tuple<int,double>>());
        auto rh = reinterpret_cast<std::vector<std::tuple<int,double>>*>(q->attrib["weight_history"]);
        rh->emplace_back(tsForHistory, qw);
    }

}

int calculateAllWeights(QuantumBackend* backend, int tsForHistory = -1)
{
    for(Component* c : *backend->GetChildren())
    {
        if(c->GetComponentType() == SYS_SAGE_COMPONENT_QUBIT)
        {
            Qubit* q = static_cast<Qubit*>(c);
            calculateQubitWeight(q, tsForHistory);
        }
    }
    return 0;
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

//calibration_data_Q-Exa_20241013.json already included in the first step
//string IQMFileNames[] = {"calibration_data_Q-Exa_20241014.json","calibration_data_Q-Exa_20241015.json","calibration_data_Q-Exa_20241016.json","calibration_data_Q-Exa_20241017.json"};
string IQMFileNames[] = {"calibration_data_Q-Exa_20241014.json","calibration_data_Q-Exa_20241015.json","calibration_data_Q-Exa_20241016.json","calibration_data_Q-Exa_20241017.json","calibration_data_Q-Exa_20241018.json","calibration_data_Q-Exa_20241019.json","calibration_data_Q-Exa_20241020.json","calibration_data_Q-Exa_20241021.json","calibration_data_Q-Exa_20241022.json","calibration_data_Q-Exa_20241023.json","calibration_data_Q-Exa_20241024.json","calibration_data_Q-Exa_20241025.json","calibration_data_Q-Exa_20241026.json","calibration_data_Q-Exa_20241027.json","calibration_data_Q-Exa_20241028.json","calibration_data_Q-Exa_20241029.json","calibration_data_Q-Exa_20241030.json","calibration_data_Q-Exa_20241031.json","calibration_data_Q-Exa_20241101.json","calibration_data_Q-Exa_20241102.json","calibration_data_Q-Exa_20241103.json","calibration_data_Q-Exa_20241104.json","calibration_data_Q-Exa_20241105.json","calibration_data_Q-Exa_20241106.json","calibration_data_Q-Exa_20241107.json","calibration_data_Q-Exa_20241108.json","calibration_data_Q-Exa_20241109.json","calibration_data_Q-Exa_20241110.json","calibration_data_Q-Exa_20241111.json","calibration_data_Q-Exa_20241112.json","calibration_data_Q-Exa_20241113.json","calibration_data_Q-Exa_20241114.json","calibration_data_Q-Exa_20241115.json","calibration_data_Q-Exa_20241116.json","calibration_data_Q-Exa_20241117.json","calibration_data_Q-Exa_20241118.json","calibration_data_Q-Exa_20241119.json","calibration_data_Q-Exa_20241120.json","calibration_data_Q-Exa_20241121.json","calibration_data_Q-Exa_20241122.json","calibration_data_Q-Exa_20241123.json","calibration_data_Q-Exa_20241124.json","calibration_data_Q-Exa_20241125.json","calibration_data_Q-Exa_20241126.json","calibration_data_Q-Exa_20241127.json","calibration_data_Q-Exa_20241128.json","calibration_data_Q-Exa_20241129.json","calibration_data_Q-Exa_20241130.json","calibration_data_Q-Exa_20241201.json","calibration_data_Q-Exa_20241202.json","calibration_data_Q-Exa_20241203.json","calibration_data_Q-Exa_20241204.json","calibration_data_Q-Exa_20241205.json"};

int main()
{
    std::cout << std::setprecision(15);
    Node* n = new Node();    
    string IQMPathPrefix = "/Users/stepan/phd/repos/q-sys-sage/tmp-qc-data/database/";
    string IQMPath;
    int IQMPathTs;
    
    IQMPath = IQMPathPrefix + "calibration_data_Q-Exa_20241013.json";
    IQMPathTs =  getTs(IQMPath);
    std::cout << "-- Parsing IQM output from file " << IQMPath << ", ts " << IQMPathTs << std::endl;
    if(parseIQM((Component*)n, IQMPath, 0, IQMPathTs) != 0) { //adds topo to a next node
        return 1;
    }
    QuantumBackend* b = static_cast<QuantumBackend*>(n->GetChild(0));
    calculateAllWeights(b,IQMPathTs);


    for(string file : IQMFileNames)
    {
        IQMPath = IQMPathPrefix + file;
        IQMPathTs =  getTs(IQMPath);
        std::cout << "-- Parsing IQM output from file " << IQMPath << ", ts " << IQMPathTs << std::endl;
        if(parseIQM(b, IQMPath, 0, IQMPathTs, false) != 0) { //adds topo to a next node
            return 1;
        }
        calculateAllWeights(b,IQMPathTs);
    }

    cout << "-- End parseIQM" << endl;

    cout << "---------------- Printing the configuration of IQM Backend----------------" << endl;
    n->PrintSubtree();
    
    for(Component * c : *b->GetChildren() )
    {
        if(c->GetComponentType() == SYS_SAGE_COMPONENT_QUBIT)
        {
            Qubit* q = static_cast<Qubit*>(c);
            std::cout << "Qubit " << q->GetId() << " has coupling map { ";
            for(Relation* r : q->relations)
            {
                if(r->GetRelationType() == SYS_SAGE_RELATION_COUPLING_MAP)
                {
                    CouplingMap* cm = static_cast<CouplingMap*>(r); 
                    std::cout << (cm->components[0] == q ? cm->components[1]->GetId() : cm->components[0]->GetId() )<< " ";
                }      
            }
            std::cout << "} and weight = " << *static_cast<double*>(q->attrib["qubit_weight"]) << "\n";
        }
    }
    

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

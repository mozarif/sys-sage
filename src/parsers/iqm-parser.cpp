/**
 * @file iqm-parser.cpp
 * @brief sys-sage's interface to IQM
 */
#include "iqm-parser.hpp"
#include <algorithm>
#include <sstream>
#include <chrono>

//using CouplingMap = std::unordered_map<int, std::vector<int>>;
//using FidelityMap = std::map<std::pair<int, int>, double>; // For storing fidelities

std::chrono::high_resolution_clock::time_point start1,start2,start3,end1,end2,end3;
int parseIQM(Component* parent, std::string dataSourcePath, int qcId, int tsForHistory)
{
    if(parent == NULL){
        std::cerr << "parseIQM: parent is null" << std::endl;
        return 1;
    }
    QuantumBackend* qc = new QuantumBackend(parent,qcId);
    return parseIQM(qc, dataSourcePath, qcId, tsForHistory, true);   
}

int parseIQM(QuantumBackend* qc, std::string dataSourcePath, int qcId, int tsForHistory, bool createTopo)
{
    start1 = std::chrono::high_resolution_clock::now();
    IQMParser iqm(qc,dataSourcePath);
    int ret;
    //only creates qubits & coupling mappings; no dynamic info
    if(createTopo)
    {
        start2 = std::chrono::high_resolution_clock::now();
        ret = iqm.CreateQcTopo();
        end2 = std::chrono::high_resolution_clock::now();
        if(ret != 0)
            return ret;
    }
    
    //assumes that the qubits and coupling mappings are already in place
    start3 = std::chrono::high_resolution_clock::now();
    ret = iqm.ParseDynamicData(tsForHistory);
    end3 = std::chrono::high_resolution_clock::now();
    end1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<long long, std::nano> duration1 = end1 - start1;
    std::chrono::duration<long long, std::nano> duration2 = end2 - start2;
    std::chrono::duration<long long, std::nano> duration3 = end3 - start3;
    std::cout << ";" << duration1.count() << ";" << duration2.count() << ";" << duration3.count();
    return ret;
}

//IQMParser::IQMParser(std::ifstream& filepath)
IQMParser::IQMParser(QuantumBackend* _qc,std::string filepath)
{
    backend = _qc;

    std::ifstream file(filepath);
    if (!file.is_open()) 
    {
        throw std::runtime_error("Failed to open file!");
    }

    jsonData = json::parse(file);
}

int IQMParser::CreateQcTopo()
{

    backend->SetName(jsonData["backend_name"]);
    int numQubits = jsonData["T1"].size();

    backend->SetNumQubits(numQubits);
    
    auto parse_pair = [](const std::string& pair_str) -> std::pair<int, int>
    {
        std::stringstream ss(pair_str);
        int x, y;
        char comma;
        ss >> x >> comma >> y;
        return {x, y};
    };

    for(int i=0; i<numQubits; i++)
    {
        Qubit* q = new Qubit(backend, i);
    }
    
    std::map<std::string, std::string> two_q_fidelity = jsonData["two_q_fidelity"];
    for(auto [cm_str,fidelity] : two_q_fidelity)
    {
        auto [q1_id, q2_id] = parse_pair(cm_str);
        std::vector<Component*>* qubits = backend->GetChildren();
        Qubit* q1 = static_cast<Qubit*>((*qubits)[q1_id]);
        if(q1->GetId() != q1_id)
            q1 = static_cast<Qubit*>(backend->GetChild(q1_id));
        Qubit* q2 = static_cast<Qubit*>((*qubits)[q2_id]);
        if(q2->GetId() != q2_id)
            q2 = static_cast<Qubit*>(backend->GetChild(q2_id));
        CouplingMap* cm = new CouplingMap(q1,q2);
    }

    //create attrib keys "T1_max", "T2_max", "q1_fidelity_max", "readout_fidelity_max", "two_q_fidelity_max"
    backend->attrib["T1_max"] = static_cast<void*>(new double());
    backend->attrib["T2_max"] = static_cast<void*>(new double());
    backend->attrib["q1_fidelity_max"] = static_cast<void*>(new double());
    backend->attrib["readout_fidelity_max"] = static_cast<void*>(new double());
    backend->attrib["two_q_fidelity_max"] = static_cast<void*>(new double());

    return 0;
}


int IQMParser::ParseDynamicData(int tsForHistory)
{
    double max;
    std::vector<double> T1;
    for (const auto& element : jsonData["T1"]) 
    {
        T1.push_back(std::stod(element.get<std::string>()));
    }
    max = *(std::max_element(T1.begin(), T1.end()));
    *static_cast<double*>(backend->attrib["T1_max"]) = max;

    std::vector<double> T2;
    for (const auto& element : jsonData["T2"]) 
    {
        T2.push_back(std::stod(element.get<std::string>()));
    }
    max = *(std::max_element(T2.begin(), T2.end()));
    *static_cast<double*>(backend->attrib["T2_max"]) = max;

    std::vector<double> q1_fidelity;
    for (const auto& element : jsonData["1q_fidelity"]) 
    {
        q1_fidelity.push_back(std::stod(element.get<std::string>()));
    }
    max = *(std::max_element(q1_fidelity.begin(), q1_fidelity.end()));
    *static_cast<double*>(backend->attrib["q1_fidelity_max"]) = max;

    std::vector<double> readout_fidelity;
    for (const auto& element : jsonData["readout_fidelity"]) 
    {
        readout_fidelity.push_back(std::stod(element.get<std::string>()));
    }
    max = *(std::max_element(readout_fidelity.begin(), readout_fidelity.end()));
    *static_cast<double*>(backend->attrib["readout_fidelity_max"]) = max;

    auto parse_pair = [](const std::string& pair_str) -> std::pair<int, int>
    {
        std::stringstream ss(pair_str);
        int x, y;
        char comma;
        ss >> x >> comma >> y;
        return {x, y};
    };

    for(int i=0; i<backend->GetNumQubits(); i++)
    {
        //Qubit *q = dynamic_cast<Qubit*>(backend->GetChildById(i));
        Qubit* q = static_cast<Qubit*>((*backend->GetChildren())[i]);
        if(q->GetId() != i)
            q = static_cast<Qubit*>(backend->GetChild(i));
        
        q->SetProperties(T1[i], T2[i], readout_fidelity[i], q1_fidelity[i]);
        if(tsForHistory > 0)
        {
            //check if readout_history exists; if not, create it -- vector of tuples <timestamp,t1,t2,readout_fidelity,q1_fidelity>
            if(! q->attrib.contains("readout_history"))
                q->attrib["readout_history"] = reinterpret_cast<void*>(new std::vector<std::tuple<int,double,double,double,double>>());
            auto rh = reinterpret_cast<std::vector<std::tuple<int,double,double,double,double>>*>(q->attrib["readout_history"]);
            rh->emplace_back(tsForHistory, T1[i], T2[i], readout_fidelity[i], q1_fidelity[i]);
        }
    }
    std::map<std::string, std::string> two_q_fidelity = jsonData["two_q_fidelity"];
    max = 0;
    for(auto [cm_str,fidelity_str] : two_q_fidelity)
    {
        auto [q1_id, q2_id] = parse_pair(cm_str);
        std::vector<Component*>* qubits = backend->GetChildren();
        Qubit* q1 = static_cast<Qubit*>((*qubits)[q1_id]);
        if(q1->GetId() != q1_id)
            q1 = static_cast<Qubit*>(backend->GetChild(q1_id));
        Qubit* q2 = static_cast<Qubit*>((*qubits)[q2_id]);
        if(q2->GetId() != q2_id)
            q2 = static_cast<Qubit*>(backend->GetChild(q2_id));
        double fidelity = std::stod(fidelity_str);
        bool cm_found = false;
        for(Relation* r: q1->relations)
        {
            if(r->GetRelationType() == SYS_SAGE_RELATION_COUPLING_MAP)
            {
                if(r->components[0] == q2 || r->components[1] == q2)
                {
                    CouplingMap* cm = static_cast<CouplingMap*>(r);
                    cm_found = true;
                    cm->SetFidelity(fidelity);
                    if(fidelity > max)
                        max = fidelity;
                            
                    if(tsForHistory > 0)
                    {
                        //check if readout_history exists; if not, create it -- vector of tuples <timestamp,fidelity>
                        if(! cm->attrib.contains("readout_history"))
                            cm->attrib["readout_history"] = reinterpret_cast<void*>(new std::vector<std::tuple<int,double>>());
                        auto rh = reinterpret_cast<std::vector<std::tuple<int,double>>*>(cm->attrib["readout_history"]);
                        rh->emplace_back(tsForHistory, fidelity);
                    }

                    break;
                }
            }
        }
        if(!cm_found)
            return 1;        
    }
    
    *static_cast<double*>(backend->attrib["two_q_fidelity_max"]) = max;


    return 0;
}

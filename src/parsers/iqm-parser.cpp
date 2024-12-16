/**
 * @file iqm-parser.cpp
 * @brief sys-sage's interface to IQM
 */
#include "iqm-parser.hpp"
#include <algorithm>
#include <sstream>

//using CouplingMap = std::unordered_map<int, std::vector<int>>;
//using FidelityMap = std::map<std::pair<int, int>, double>; // For storing fidelities


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
    IQMParser iqm(qc,dataSourcePath);
    int ret;
    //only creates qubits & coupling mappings; no dynamic info
    if(createTopo)
    {
        ret = iqm.CreateQcTopo();
        if(ret != 0)
            return ret;
    }
    
    //assumes that the qubits and coupling mappings are already in place
    ret = iqm.ParseDynamicData(tsForHistory);
    if(ret != 0)
        return ret;
    
    ret = iqm.CalculateAllWeights(tsForHistory);

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
    std::cout << "   [sys-sage]...............Initiated "
                  << "IQM session\n";
}

int IQMParser::CalculateAllWeights(int tsForHistory)
{
    for(Component* c : *backend->GetChildren())
    {
        if(c->GetComponentType() == SYS_SAGE_COMPONENT_QUBIT)
        {
            Qubit* q = static_cast<Qubit*>(c);
            q->CalculateWeight(t1_max, t2_max, q1_fidelity_max, readout_fidelity_max, tsForHistory);
        }
    }
    return 0;
}

int IQMParser::ParseDynamicData(int tsForHistory)
{
    std::vector<double> T1;
    for (const auto& element : jsonData["T1"]) 
    {
        T1.push_back(std::stod(element.get<std::string>()));
    }
    t1_max = *(std::max_element(T1.begin(), T1.end()));

    std::vector<double> T2;
    for (const auto& element : jsonData["T2"]) 
    {
        T2.push_back(std::stod(element.get<std::string>()));
    }
    t2_max = *(std::max_element(T2.begin(), T2.end()));

    std::vector<double> q1_fidelity;
    for (const auto& element : jsonData["1q_fidelity"]) 
    {
        q1_fidelity.push_back(std::stod(element.get<std::string>()));
    }
    q1_fidelity_max = 1;

    std::vector<double> readout_fidelity;
    for (const auto& element : jsonData["readout_fidelity"]) 
    {
        readout_fidelity.push_back(std::stod(element.get<std::string>()));
    }
    readout_fidelity_max = 1;


    // std::cout << "t1_max: " << t1_max << "\n";
    // std::cout << "t2_max: " << t2_max << "\n";
    // std::cout << "q1_fidelity_max: " << q1_fidelity_max << "\n";
    // std::cout << "readout_max: " << readout_fidelity_max << "\n";

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
        Qubit *q = dynamic_cast<Qubit*>(backend->GetChildById(i));
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
    for(auto [cm_str,fidelity_str] : two_q_fidelity)
    {
        auto [q1_id, q2_id] = parse_pair(cm_str);
        Qubit* q1 = static_cast<Qubit*>(backend->GetChild(q1_id));
        Qubit* q2 = static_cast<Qubit*>(backend->GetChild(q2_id));
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

    for(Component* q : *backend->GetChildren())
    {
        //std::cout << "Qubit " << q->GetId() << ":" << std::endl;
        for(Relation* r : q->relations)
        {
            if(r->GetRelationType() == SYS_SAGE_RELATION_COUPLING_MAP)
            {
                CouplingMap* cm = static_cast<CouplingMap*>(r);
                //cout << "    " << cm->components[0]->GetId() << " -> " << cm->components[1]->GetId() << " : " << cm->GetFidelity() << std::endl;
            }
        }
    }

    return 0;
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
        Qubit* q1 = static_cast<Qubit*>(backend->GetChild(q1_id));
        Qubit* q2 = static_cast<Qubit*>(backend->GetChild(q2_id));
        CouplingMap* cm = new CouplingMap(q1,q2);
        // cout << "qubit " << q1_id << " has relations: " << q1->relations.size() << std::endl;
        // cout << "qubit " << q2_id << " has relations: " << q2->relations.size() << std::endl;
    }

    // for(Component* q : *backend->GetChildren())
    // {
    //     //std::cout << "Qubit " << q->GetId() << ":" << std::endl;
    //     for(Relation* r : q->relations)
    //     {
    //    //     cout << "    " << r->components[0]->GetId() << " -> " << r->components[1]->GetId() << std::endl;
    //     }
    // }
    return 0;
}


// void IQMParser::CreateQcTopo(QuantumBackend *backend)
// {
//     setQubits(backend);
// }

// QuantumBackend IQMParser::CreateQcTopo(int device_index, std::string device_name)
// {
//     QuantumBackend qc = QuantumBackend(device_index, device_name);
//     CreateQcTopo(&qc);

//     return qc;
// }

// QuantumBackend IQMParser::CreateQcTopo()
// {
//     std::string name = _data["backend_name"];
//     QuantumBackend qc = QuantumBackend(0, name);
//     CreateQcTopo(&qc);

//     return qc;
// }
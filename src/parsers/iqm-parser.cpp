/**
 * @file iqm-parser.cpp
 * @brief sys-sage's interface to IQM
 */

#include "iqm-parser.hpp"

using CouplingMap = std::unordered_map<int, std::vector<int>>;
using FidelityMap = std::map<std::pair<int, int>, double>; // For storing fidelities

IQMParser::IQMParser(std::ifstream& filepath)
{ 
    if (!filepath.is_open()) 
    {
        throw std::runtime_error("Failed to open file!");
    }

    _data = json::parse(filepath);
    std::cout << "   [sys-sage]...............Initiated "
                  << "IQM session\n";
}

void IQMParser::setQubits(QuantumBackend *backend)
{
    std::vector<double> T1;
    for (const auto& element : _data["T1"]) 
    {
        T1.push_back(std::stod(element.get<std::string>()));
    }

    std::vector<double> T2;
    for (const auto& element : _data["T2"]) 
    {
        T2.push_back(std::stod(element.get<std::string>()));
    }

    std::vector<double> q1_fidelity;
    for (const auto& element : _data["1q_fidelity"]) 
    {
        q1_fidelity.push_back(std::stod(element.get<std::string>()));
    }

    std::vector<double> readout_fidelity;
    for (const auto& element : _data["readout_fidelity"]) 
    {
        readout_fidelity.push_back(std::stod(element.get<std::string>()));
    }

    backend->SetNumberofQubits(T1.size());

    auto two_q_fidelity = _data["two_q_fidelity"];
    auto parse_pair = [](const std::string& pair_str) -> std::pair<int, int>
    {
        std::stringstream ss(pair_str);
        int x, y;
        char comma;
        ss >> x >> comma >> y;
        return {x, y};
    };

    // Save coupling map and 2q fidelities
    CouplingMap coupling_map;
    FidelityMap fidelity_map;

    for (auto it = two_q_fidelity.begin(); it != two_q_fidelity.end(); ++it)
    {
        std::string pair_str = it.key();
        double fidelity = std::stod(it.value().get<std::string>());

        auto [q1, q2] = parse_pair(pair_str);

        // Update the coupling map
        coupling_map[q1].push_back(q2);
        coupling_map[q2].push_back(q1);

        // Store the fidelity
        fidelity_map[{q1, q2}] = fidelity;
        fidelity_map[{q2, q1}] = fidelity;
    }

    for ( auto i = 0; i < backend->GetNumberofQubits(); i++)
    {
        Qubit* q = new Qubit(backend, i);
        q->SetProperties(T1[i], T2[i], readout_fidelity[i], q1_fidelity[i]);

        std::vector <Qubit::NeighbouringQubit> cmp;
        for (auto qubit : coupling_map[i])
        {
            double fidelity = fidelity_map[{i, qubit}];
            cmp.emplace_back(Qubit::NeighbouringQubit(qubit, fidelity));
        }
        
        q->SetCouplingMapping(cmp, coupling_map[i].size());
    }

    return;
}

void IQMParser::CreateQcTopo(QuantumBackend *backend)
{
    setQubits(backend);
}

QuantumBackend IQMParser::CreateQcTopo(int device_index, std::string device_name)
{
    QuantumBackend qc = QuantumBackend(device_index, device_name);
    CreateQcTopo(&qc);

    return qc;
}

QuantumBackend IQMParser::CreateQcTopo()
{
    std::string name = _data["backend_name"];
    QuantumBackend qc = QuantumBackend(0, name);
    CreateQcTopo(&qc);

    return qc;
}
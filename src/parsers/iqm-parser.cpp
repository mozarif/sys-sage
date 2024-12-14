/**
 * @file iqm-parser.cpp
 * @brief sys-sage's interface to IQM
 */

#include "iqm-parser.hpp"


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

    for ( auto i = 0; i < backend->GetNumberofQubits(); i++)
    {
        Qubit* q = new Qubit(backend, i);
        q->SetProperties(T1[i], T2[i],readout_fidelity[i],q1_fidelity[i]);
    }

    return;

}

void IQMParser::createQcTopo(QuantumBackend *backend)
{   
    //backend->SetNumberofQubits(get_num_qubits(dev));
    setQubits(backend);
}

QuantumBackend IQMParser::createQcTopo(int device_index, std::string device_name)
{
    QuantumBackend qc = QuantumBackend(device_index, device_name);
    createQcTopo(&qc);

    return qc;
}

QuantumBackend IQMParser::createQcTopo()
{
    std::string name = _data["backend_name"];
    QuantumBackend qc = QuantumBackend(0, name);
    createQcTopo(&qc);

    return qc;
}
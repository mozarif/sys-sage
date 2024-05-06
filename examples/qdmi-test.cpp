/* Possible exports
export CPATH=$CPATH:/home/diogenes/sys-sage/build/_deps/qinfo-src/include/
export CPATH=$CPATH:/home/diogenes/sys-sage/build/_deps/qdmi-src/include
export CONF_IBM=/home/diogenes/qdmi.git/inputs/conf.json
export QDMI_CONFIG_FILE=/home/diogenes/sys-sage/build/_deps/qdmi-src/.qdmi-config
export PROP_IBM=/home/diogenes/qdmi.git/inputs/props.json
*/

#include <iostream>
#include <string.h>
#include <iomanip>
#include "sys-sage.hpp"

int main()
{
    std::cout << std::setprecision(15);
    // Create an instance of the interface
    QDMI_Parser qdmi;

    // Use QDMI_Parser to create the topology of either all the backends or one of the backends
    /*******************************************Method 1**********************************************/

    Topology* qc_topo = new Topology();
    qdmi.createQcTopo(qc_topo);

    cout << "---------------- Printing the configuration of IBM Backend----------------" << endl;
    qc_topo->PrintSubtree();
    cout << "---------------- Printing Qubit Coupling Mappings for IBM Backend----------------" << endl;
    QuantumBackend* qc = dynamic_cast<QuantumBackend*>(qc_topo->GetChild(0));
    int total_qubits = qc->CountAllSubcomponents();
    for (int i = 0; i < total_qubits; i++)
    {
        Qubit* q = dynamic_cast<Qubit*>(qc->GetChild(i));
        std::cout << "Qubit " << i << " has coupling map { ";
        auto coupling_map = q->GetCouplingMapping();
        for (long unsigned j = 0; j < coupling_map.size(); j++)
        {
            std::cout << coupling_map[j] << " ";
        }
        std::cout << "}\n";
    }
    cout << "---------------- Printing Supported Gate Types for IBM Backend----------------" << endl;
    int num_gates = qc->GetNumberofGates();
    auto gates = qc->GetGateTypes();
    for (int i = 0; i < num_gates; ++i)
    {   
        std::cout << gates[i] << "\n";
    }

    std::cout << "---------------- Printing Qubit Properties for IBM Backend----------------" << endl;
    for (int i = 0; i < total_qubits; i++)
    {
        Qubit* q = dynamic_cast<Qubit*>(qc->GetChild(i));
        std::cout << "Qubit " << i << " has following properties: \n";
        std::cout << "      T1: " << q->GetT1() << "\n";
        std::cout << "      T2: " << q->GetT2() << "\n";
        std::cout << "      Readout Error: " << q->GetReadoutError() << "\n";
        std::cout << "      Readout Length: " << q->GetReadoutLength() << "\n";
    }

    /*******************************************Method 2**********************************************/
    // auto quantum_backends = qdmi.get_available_backends();
    // std::cout << "Total " << quantum_backends.size() << " devices found.\n";
    // QuantumBackend* qc_topo = new QuantumBackend(0, quantum_backends[0].first);
    // qdmi.createQcTopo(qc_topo, quantum_backends[0].second);

    // cout << "---------------- Printing the configuration of QLM Backend ----------------" << endl;
    // qc_topo->PrintSubtree();
    // cout << "---------------- Printing Qubit Coupling Mappings of QLM Backend----------------" << endl;
    // int total_qubits = qc_topo->CountAllSubcomponents();
    // for (int i = 0; i < total_qubits; i++)
    // {
    //     Qubit* q = dynamic_cast<Qubit*>(qc_topo->GetChild(i));
    //     std::cout << "Qubit " << i << " has coupling map { ";
    //     auto coupling_map = q->GetCouplingMapping();
    //     for (long unsigned j = 0; j < coupling_map.size(); j++)
    //     {
    //         std::cout << coupling_map[j] << " ";
    //     }
    //     std::cout << "}\n";
    // }
       
    return 0;
}

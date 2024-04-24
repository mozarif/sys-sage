/* Possible exports
export CPATH=$CPATH:/home/diogenes/sys-sage/build/_deps/qinfo-src/include/
export CPATH=$CPATH:/home/diogenes/sys-sage/build/_deps/qdmi-src/include
export CONF_IBM=/home/diogenes/qdmi.git/inputs/conf.json
export QDMI_CONFIG_FILE=/home/diogenes/sys-sage/build/_deps/qdmi-src/.qdmi-config
*/

#include <iostream>
#include <string.h>
#include "sys-sage.hpp"

int main()
{

    // Create an instance of the interface
    QDMI_Interface qdmi;

    // Use QDMI_Interface to create the topology of either all the backends or one of the backends
    /*******************************************Method 1**********************************************/

    Topology* qc_topo = new Topology();
    qdmi.createQcTopo(qc_topo);

    cout << "---------------- Printing the configuration of QLM Backend----------------" << endl;
    qc_topo->PrintSubtree();
    cout << "---------------- Printing Qubit Coupling Mappings for QLM Backend----------------" << endl;
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
    cout << "---------------- Printing Supported Gate Types for QLM Backend----------------" << endl;
    int num_gates = qc->GetNumberofGates();
    auto gates = qc->GetGateTypes();
    for (int i = 0; i < num_gates; ++i)
    {   
        std::cout << gates[i] << "\n";
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

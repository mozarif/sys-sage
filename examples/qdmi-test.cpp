#include <iostream>
#include <string.h>
#include "sys-sage.hpp"

#define CHECK_ERR(a,b) { if (a!=QDMI_SUCCESS) { printf("\n[Error]: %i at %s",a,b); return 1; }}

int main()
{
    int err = 0;
    QInfo info = NULL;
    QDMI_Session session = NULL;

    err = QInfo_create(&info);
    CHECK_ERR(err, "QInfo_create");

    err = QDMI_session_init(info, &session);
    CHECK_ERR(err, "QDMI_session_init");

    QDMI_Interface qdmi;
    auto vec = qdmi.get_available_backends();
    std::cout << "Total " << vec.size() << " devices found.\n";
    
    //create root quantum backend
    QuantumBackend* qc = new QuantumBackend();
    int num_qubits = qdmi.get_num_qubits(vec[vec.size()-1]); // Using Device index = vec.size()-1

    if(num_qubits == -1)
    {
        std::cout << "Error in getting number of qubits\n";
        return 1;
    }

    qdmi.set_qubits(vec[vec.size()-1], vec.size()-1); // Using Device index = vec.size()-1

    for (int i = 0; i < num_qubits; i++)
    {
        Qubit* q = new Qubit(qc, i);
        qdmi.setCouplingMapping(q, vec.size()-1, i); // Using Device index = vec.size()-1
        auto coupling_map = q->GetCouplingMapping();
    }

    cout << "---------------- Printing Qubit Coupling Mappings ----------------" << endl;
    int total_qubits = qc->CountAllSubcomponents();
    for (int i = 0; i < total_qubits; i++)
    {
        // TODO: Dynamic cast or static cast?
        Qubit* q = dynamic_cast<Qubit*>(qc->GetChild(i));
        std::cout << "Qubit " << i << " has coupling map { ";
        auto coupling_map = q->GetCouplingMapping();
        for (long unsigned j = 0; j < coupling_map.size(); j++)
        {
            std::cout << coupling_map[j] << " ";
        }
        std::cout << "}\n";
    }
    
    cout << "---------------- Printing the configuration of QC ----------------" << endl;
    qc->PrintSubtree();
    
    return 0;
}

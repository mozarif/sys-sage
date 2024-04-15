#include <iostream>
#include <string.h>
#include "sys-sage.hpp"

#define CHECK_ERR(a,b) { if (a!=QDMI_SUCCESS) { printf("\n[Error]: %i at %s",a,b); return 1; }}

int main()
{
    int err = 0, status = 0;
    int index = 0;
    int n_shots = 100;
    QInfo info = NULL;
    QDMI_Session session = NULL;
    QDMI_Job job;
    QDMI_Fragment frag = (QDMI_Fragment) malloc(sizeof(QDMI_Fragment));
    QDMI_Device device = (QDMI_Device) malloc(sizeof(QDMI_Device));

    err = QInfo_create(&info);
    CHECK_ERR(err, "QInfo_create");

    err = QDMI_session_init(info, &session);
    CHECK_ERR(err, "QDMI_session_init");

    auto vec = get_available_backends();
    int count = 0;
    std::cout << "Total " << vec.size() << " devices found.\n";
    // for (auto dev : vec)
    // {
    //     std::cout << "Printing for device id: " << count++ << "\n";
    //     print_coupling_mappings(dev);
    // }

    //create root quantum backend
    QuantumBackend* qc = new QuantumBackend();
    int num_qubits = get_num_qubits(vec[vec.size()-1]);

    if(num_qubits == -1)
    {
        std::cout << "Error in getting number of qubits\n";
        return 1;
    }

    set_qubits(vec[vec.size()-1], vec.size()-1);

    for (int i = 0; i < num_qubits; i++)
    {
        Qubit* q = new Qubit(qc, i);
        setCouplingMapping(q, vec.size()-1, i);
    }
    

    cout << "---------------- Printing the configuration of QC ----------------" << endl;
    qc->PrintSubtree();
    
    return 0;
}

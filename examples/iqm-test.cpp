#include <iomanip>
#include "sys-sage.hpp"

int main()
{
    std::cout << std::setprecision(15);
    std::ifstream filePath("/home/diogenes/sys-sage/database/calibration_data_Q-Exa_20241013.json");
    IQMParser iqm(filePath);

    auto qc = iqm.CreateQcTopo();

    cout << "---------------- Printing the configuration of IQM Backend----------------" << endl;
    qc.PrintSubtree();

    int total_qubits = qc.CountAllSubcomponents();
    for (int i = 0; i < total_qubits; i++)
    {
        Qubit* q = dynamic_cast<Qubit*>(qc.GetChild(i));
        // std::cout << "Qubit " << i << " has following T1, T2, readout_fidelity and 1q_fidelity: ";
        // std::cout << q->GetT1() << ", " << q->GetT2() << ", " << q->GetReadoutFidelity() << ", " << q->Get1QFidelity() << "\n";
        //Qubit* q = dynamic_cast<Qubit*>(qc->GetChild(i));
        std::cout << "Qubit " << i << " has coupling map { ";
        auto coupling_map = q->GetCouplingMapping();
        for (long unsigned j = 0; j < coupling_map.size(); j++)
        {
            std::cout << coupling_map[j]._qubit_index << "(" << coupling_map[j]._fidelity << ") ";
        }
        std::cout << "}\n";
    }

    return 0;
}

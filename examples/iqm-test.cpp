#include <iomanip>
#include "sys-sage.hpp"

int main()
{
    std::cout << std::setprecision(15);


    string IQMPath;
    IQMPath = "/Users/stepan/phd/repos/q-sys-sage/tmp-qc-data/database/calibration_data_Q-Exa_20241013.json";
    
    Node* n = new Node();    
    cout << "-- Parsing IQM output from file " << IQMPath << endl;
    if(parseIQM((Component*)n, IQMPath, 0) != 0) { //adds topo to a next node
        return 1;
    }
    cout << "-- End parseIQM" << endl;

    cout << "---------------- Printing the configuration of IQM Backend----------------" << endl;
    qc.PrintSubtree();

    // std::ifstream filePath("/Users/stepan/phd/repos/q-sys-sage/tmp-qc-data/database/calibration_data_Q-Exa_20241013.json");
    // //std::ifstream filePath("/home/diogenes/sys-sage/database/calibration_data_Q-Exa_20241013.json");
    // IQMParser iqm(filePath);

    // auto qc = iqm.CreateQcTopo();

    // cout << "---------------- Printing the configuration of IQM Backend----------------" << endl;
    // qc.PrintSubtree();

    // int total_qubits = qc.CountAllSubcomponents();
    // std::vector <int> neighbour_count;
    // std::vector<double> weights; 

    // neighbour_count.resize(total_qubits);
    // weights.resize(total_qubits);
    // for (int i = 0; i < total_qubits; i++)
    // {
    //     Qubit* q = dynamic_cast<Qubit*>(qc.GetChild(i));
    //     // std::cout << "Qubit " << i << " has following T1, T2, readout_fidelity and 1q_fidelity: ";
    //     // std::cout << q->GetT1() << ", " << q->GetT2() << ", " << q->GetReadoutFidelity() << ", " << q->Get1QFidelity() << "\n";
    //     //Qubit* q = dynamic_cast<Qubit*>(qc->GetChild(i));
    //     std::cout << "Qubit " << i << " has coupling map { ";
    //     auto coupling_map = q->GetCouplingMapping();
    //     neighbour_count[i] = coupling_map.size();
    //     for (long unsigned j = 0; j < coupling_map.size(); j++)
    //     {
    //         std::cout << coupling_map[j]._qubit_index << " ";
    //     }
    //     weights[i] = q->GetWeight();
    //     std::cout << "} and weight = " << q->GetWeight() << "\n";
    // }

    // std::ofstream file("output.csv");
    // if (!file.is_open()) 
    // {
    //     std::cerr << "Failed to open file for writing." << std::endl;
    //     return 1;
    // }

    // file << std::fixed << std::setprecision(14);

    // // Write data to the CSV file
    // for (size_t i = 0; i < neighbour_count.size(); ++i) 
    // {
    //     file << neighbour_count[i] << "," << weights[i] << "\n";
    // }

    // // Close the file
    // file.close();

    return 0;
}

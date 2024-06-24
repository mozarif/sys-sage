/**
* @file qdmi-parser.hpp
* @brief sys-sage's interface to QDMI.
*/

#ifndef QDMI_Parser_HPP
#define QDMI_Parser_HPP

#include <iostream>
#include <string.h>
#include <vector>
#include <map>
#include "Topology.hpp"
#include <qdmi.h>
#include <qdmi_internal.h>

#define CHECK_ERR(a,b) { if (a!=QDMI_SUCCESS) { printf("\n[Error]: %i at %s",a,b); return 1; }}

extern "C"
{
    class QDMI_Parser
    {
    public:
        QDMI_Parser();
        std::vector<std::pair <std::string, QDMI_Device>> get_available_backends();

        int get_num_qubits(QDMI_Device dev);
        void createQcTopo(Topology *topo);
        void createQcTopo(QuantumBackend *topo, QDMI_Device dev);
        
        /**********TODO: remove this*****************************/ 
        void set_qubits(QDMI_Device dev, int device_index);
        /*********************************************************/
        static void refreshQubitProprties(QuantumBackend *qc, Qubit *qubit)
        {
            //     //auto quantum_backends = get_available_backends();

            //     // Search for required QuantumBackend in the device and call these:
            //     //QDMI_Device dev = qc->
            //     //QDMI_query_qubit_property(QDMI_Device dev, QDMI_Qubit_property prop, QDMI_Qubit qubit, int* coupling_map);
        }
    private:
        int initiateSession();
        static QInfo info;
        static QDMI_Session session;
        void getCouplingMapping(QDMI_Device dev, QDMI_Qubit qubit, std::vector<int> &coupling_mapping, int &coupling_map_size);
        void getQubitProperties(QDMI_Device dev, QDMI_Qubit qubit);
        void setQubits(QuantumBackend *backend, QDMI_Device dev);
        void setGateSets(QuantumBackend *backend, QDMI_Device dev);
        //TODO: Is this needed?
        std::map < int, std::vector <QDMI_Qubit_impl_d >> _qubits;
    };
    
}

#endif // QDMI_Parser_HPP
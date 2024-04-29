/**
 * @file qdmi-interface.hpp
 * @brief sys-sage's interface to QDMI. Based on MQSS's global FOMAC.
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
        int initiateSession();
        std::vector<std::pair <std::string, QDMI_Device>> get_available_backends();

        int get_num_qubits(QDMI_Device dev);
        /**********Maybe remove these*****************************/ 
        void set_qubits(QDMI_Device dev, int device_index);
        void setCouplingMapping(Qubit *_qubit, int device_index, int qubit_index);
        /*********************************************************/
        void setQubits(QuantumBackend *backend, QDMI_Device dev);
        void setGateSets(QuantumBackend *backend, QDMI_Device dev);
        void createQcTopo(Topology *topo);
        void createQcTopo(QuantumBackend *topo, QDMI_Device dev);
        static void refreshQubitProprties(QuantumBackend *qc, Qubit *qubit)
        {
            //     //auto quantum_backends = get_available_backends();

            //     // Search for required QuantumBackend in the device and call these:
            //     //QDMI_Device dev = qc->
            //     //QDMI_query_qubit_property(QDMI_Device dev, QDMI_Qubit_property prop, QDMI_Qubit qubit, int* coupling_map);
        }
    private:
        static QInfo info;
        static QDMI_Session session;
        //TODO: Is this needed?
        std::map < int, std::vector <QDMI_Qubit_impl_d >> _qubits;
    };
    
}

#endif // QDMI_Parser_HPP
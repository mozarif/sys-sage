/**
 * @file qdmi-interface.hpp
 * @brief sys-sage's interface to QDMI. Based on MQSS's global FOMAC.
 */

#ifndef QDMI_INTERFACE_HPP
#define QDMI_INTERFACE_HPP

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
    class QDMI_Interface
    {
    public:
        QDMI_Interface();
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
    private:
        QInfo info;
        QDMI_Session session;
        //TODO: Is this needed?
        std::map < int, std::vector <QDMI_Qubit_impl_d >> _qubits;
    };
    
}

#endif // QDMI_INTERFACE_HPP
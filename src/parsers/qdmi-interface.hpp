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


extern "C"
{
    class QDMI_Interface
    {
    public:
        std::vector<QDMI_Device> get_available_backends();
        int get_num_qubits(QDMI_Device dev);
        void set_qubits(QDMI_Device dev, int device_index);
        void get_qubit(QDMI_Device dev, int index);
        void print_coupling_mappings(QDMI_Device dev);
        void setCouplingMapping(Qubit *_qubit, int device_index, int qubit_index);
    private:
        //TODO: Try storing the QDMI_Qubit instead of QDMI_Qubit_impl_d
        std::map < int, std::vector <QDMI_Qubit_impl_d >> _qubits;
    };
    
}

#endif // QDMI_INTERFACE_HPP
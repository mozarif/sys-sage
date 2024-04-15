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

/**
 * @brief TODO
 * @return TODO
 * @todo To be implemented
 */
extern "C"
{
    std::vector<QDMI_Device> get_available_backends();
}


/**
 * @brief TODO
 * @return TODO
 * @todo To be implemented
 */
extern "C" 
{
    int get_num_qubits(QDMI_Device dev);
}

/**
 * @brief TODO
 * @return TODO
 * @todo To be implemented
 */
extern "C" 
{
    void set_qubits(QDMI_Device dev, int device_index);
}

extern "C"
{
    void setCouplingMapping(Qubit *_qubit, int device_index, int qubit_index);
}


/**
 * @brief TODO
 * @return TODO
 * @todo To be implemented
 */
extern "C"
{
    void get_qubit(QDMI_Device dev, int index);
}

/**
 * @brief TODO
 * @return TODO
 * @todo To be implemented
 */
extern "C"
{
    void print_coupling_mappings(QDMI_Device dev);
}

#endif // QDMI_INTERFACE_HPP
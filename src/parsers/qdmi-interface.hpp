/**
 * @file qdmi-interface.hpp
 * @brief sys-sage's interface to QDMI. Based on MQSS's global FOMAC.
 */

#ifndef QDMI_INTERFACE_HPP
#define QDMI_INTERFACE_HPP

#include <iostream>
#include <string.h>
#include <vector>

#include <qdmi.h>
#include <qdmi_internal.h>

//const char *registered_devices[] = {
//    "/bin/lib/libbackend_qlm.so",
//    "/bin/lib/libbackend_ibm.so",
//    "/bin/lib/libbackend_wmi.so",
//    "/bin/lib/libbackend_q7.so",
//};

/**
 * @brief TODO
 * @return TODO
 * @todo To be implemented
 */
extern "C"
{
    std::vector<QDMI_Device> FOMAC_available_devices();
}

/**
 * @brief TODO
 * @return TODO
 * @todo To be implemented
 */
extern "C"
{
    void FOMAC_print_coupling_mappings(QDMI_Device dev);
}

#endif // QDMI_INTERFACE_HPP
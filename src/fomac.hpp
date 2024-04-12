/**
 * @file fomac.hpp
 * @brief Header of the global FoMaC.
 */

#ifndef FOMAC_HPP
#define FOMAC_HPP

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

#endif // FOMAC_HPP
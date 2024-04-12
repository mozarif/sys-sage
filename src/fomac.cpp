/**
 * @file fomac.cpp
 * @brief Implementation of global FoMaC.
 */

#include "fomac.hpp"

/**
 * @brief TODO
 * @return TODO
 * @todo Should this become a daemon? In which node should it run?
 */
extern "C" std::vector<QDMI_Device> FOMAC_available_devices()
{
    // TODO REPEAT PERIODICALLY WITH A DELAY

    char **available_devices = get_qdmi_library_list_names();
    if (available_devices == NULL)
    {
        std::cout << "   [FoMaC]...............Failed to get "
                  << "qdmi_library_list from QDMI" << std::endl;

        return std::vector<QDMI_Device>();
    }

    int i = 0, err = 0, status = 0;
    std::vector<QDMI_Device> registered_devices;
    while (available_devices[i] != NULL)
    {
        QDMI_Device device =
            (QDMI_Device)malloc(sizeof(struct QDMI_Device_impl_d));

        QDMI_Library lib = find_library_by_name(available_devices[i]);

        if (!lib)
        {
            std::cout << "   [FoMaC]...............A backend's library "
                      << "could not be found" << std::endl;

            free(available_devices[i++]);
            continue;
        }

        device->library = *lib;

        // TODO HANDLE err
        err = QDMI_device_status(device, device->library.info, &status);

        if (status)
        {
            const char *device_name = strrchr(available_devices[i], '/');
            std::cout << "   [FoMaC]...............Available device "
                      << "found: " << device_name << std::endl;

            registered_devices.push_back(device);
        }

        free(available_devices[i++]);
    }
    free(available_devices);

    return registered_devices;
}

/**
 * @brief TODO
 * @return TODO
 * @todo Should this become a daemon? In which node should it run?
 */
extern "C" void FOMAC_print_coupling_mappings(QDMI_Device dev)
{
    // TODO REPEAT PERIODICALLY WITH A DELAY

    QDMI_Qubit qubits;
    int err, num_qubits = 0;

    err = QDMI_query_all_qubits(dev, &qubits);

    if (err != QDMI_SUCCESS || qubits == NULL)
    {
        std::cout << "   [FoMaC]...............Could not obtain available "
                  << "qubits via QDMI\n";
        return;
    }

    err = QDMI_query_qubits_num(dev, &num_qubits);

    if (err != QDMI_SUCCESS || num_qubits == 0)
    {
        std::cout << "   [FoMaC]...............Could not obtain number of "
                  << "available qubits via QDMI\n";
        return;
    }

    for (int i = 0; i < num_qubits; i++)
    {
        if (qubits[i].coupling_mapping == NULL)
        {
            std::cout << "   [FoMaC]...............No coupling mapping"
                      << std::endl;
            continue;
        }

        std::cout << "   [FoMaC]...............Coupling mapping of qubit[" << i
                  << "]: { ";
        for (int j = 0; j < qubits[i].size_coupling_mapping; j++)
            std::cout << qubits[i].coupling_mapping[j] << " ";
        std::cout << "}" << std::endl;
    }

    free(qubits);

    return;
}
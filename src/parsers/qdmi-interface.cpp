/**
 * @file qdmi-interface.hpp
 * @brief sys-sage's interface to QDMI. Based on MQSS's global FOMAC.
 */

#include "qdmi-interface.hpp"


// TODO: Change function name (like FOMAC_available_devices)
extern "C" std::vector<QDMI_Device> QDMI_Interface::get_available_backends()
{
    // TODO REPEAT PERIODICALLY WITH A DELAY

    char **available_devices = get_qdmi_library_list_names();
    if (available_devices == NULL)
    {
        std::cout << "   [sys-sage]...............Failed to get "
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
            std::cout << "   [sys-sage]...............A backend's library "
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
            std::cout << "   [sys-sage]...............Available device "
                      << "found: " << device_name << std::endl;

            registered_devices.push_back(device);
        }

        free(available_devices[i++]);
    }
    free(available_devices);

    return registered_devices;
}

extern "C" int QDMI_Interface::get_num_qubits(QDMI_Device dev)
{
    int err, num_qubits = 0;

    err = QDMI_query_qubits_num(dev, &num_qubits);
    
    if (err != QDMI_SUCCESS || num_qubits == 0)
    {
        std::cout << "   [sys-sage]...............Could not obtain number of "
                  << "available qubits via QDMI\n";
        return -1;
    }
    return num_qubits;

}

extern "C" void QDMI_Interface::set_qubits(QDMI_Device dev, int device_index)
{
    QDMI_Qubit qubits;
    int err, num_qubits = 0;

    err = QDMI_query_all_qubits(dev, &qubits);

    if (err != QDMI_SUCCESS || qubits == NULL)
    {
        std::cout << "   [sys-sage]...............Could not obtain available "
                  << "qubits via QDMI\n";
        return;
    }

    err = QDMI_query_qubits_num(dev, &num_qubits);
    
    if (err != QDMI_SUCCESS || num_qubits == 0)
    {
        std::cout << "   [sys-sage]...............Could not obtain number of "
                  << "available qubits via QDMI\n";
        return;
    }
    else 
    {
        std::cout << "   [sys-sage]...............Found "
                  << num_qubits << " qubits.\n";
    }

    _qubits[device_index] = std::vector <QDMI_Qubit_impl_d >();

    for(int i = 0; i < num_qubits; ++i)
    {
        _qubits[device_index].push_back(qubits[i]);
    }

    free(qubits);

    return;

}

extern "C" void QDMI_Interface::setCouplingMapping(Qubit *_qubit, int device_index, int qubit_index)
{

    if (_qubits.find(device_index) == _qubits.end())
    {
        std::cout << "   [sys-sage]...............No qubits found for device "
                  << device_index << std::endl;
        return;
    }


    auto qubit_impl = _qubits[device_index][qubit_index];
    // if (!qubit_impl)
    // {
    //     std::cout << "   [sys-sage]...............Qubit is null for device "
    //               << device_index << " and qubit index " << qubit_index << std::endl;
    //     return;
    // }

    int size = qubit_impl.size_coupling_mapping; //qubit_impl->size_coupling_mapping;
    if (size <= 0)
    {
        std::cout << "   [sys-sage]...............Invalid size of coupling mapping for qubit "
                  << "with device index " << device_index << " and qubit index " << qubit_index << std::endl;
        return;
    }


    std::vector<int> coupling_mapping(size);

    // Copy the coupling_mapping from qubit_impl
    std::copy(qubit_impl.coupling_mapping, qubit_impl.coupling_mapping + size, coupling_mapping.begin());

    _qubit->SetCouplingMapping(coupling_mapping, size);

    return;
}

// TODO: change function name (like FOMAC_print_coupling_mappings)
extern "C" void QDMI_Interface::print_coupling_mappings(QDMI_Device dev)
{
    // TODO REPEAT PERIODICALLY WITH A DELAY

    QDMI_Qubit qubits;
    int err, num_qubits = 0;

    err = QDMI_query_all_qubits(dev, &qubits);

    if (err != QDMI_SUCCESS || qubits == NULL)
    {
        std::cout << "   [sys-sage]...............Could not obtain available "
                  << "qubits via QDMI\n";
        return;
    }

    err = QDMI_query_qubits_num(dev, &num_qubits);
    
    if (err != QDMI_SUCCESS || num_qubits == 0)
    {
        std::cout << "   [sys-sage]...............Could not obtain number of "
                  << "available qubits via QDMI\n";
        return;
    }
    else 
    {
        std::cout << "   [sys-sage]...............Found "
                  << num_qubits << " qubits.\n";
    }

    for (int i = 0; i < num_qubits; i++)
    {
        if (qubits[i].coupling_mapping == NULL)
        {
            std::cout << "   [sys-sage]...............No coupling mapping"
                      << std::endl;
            continue;
        }

        std::cout << "   [sys-sage]...............Coupling mapping of qubit[" << i
                  << "]: { ";
        for (int j = 0; j < qubits[i].size_coupling_mapping; j++)
            std::cout << qubits[i].coupling_mapping[j] << " ";
        std::cout << "}" << std::endl;
    }

    free(qubits);

    return;
}
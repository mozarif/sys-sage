/**
 * @file qdmi-parser.hpp
 * @brief sys-sage's interface to QDMI. Based on MQSS's global FOMAC.
 */

#include "qdmi-parser.hpp"

QInfo QDMI_Parser::info; 
QDMI_Session QDMI_Parser::session; 

extern "C" QDMI_Parser::QDMI_Parser()
{
    initiateSession();
}

extern "C" int QDMI_Parser::initiateSession()
{
    int err = 0;
    QDMI_Parser::info = NULL;
    QDMI_Parser::session = NULL;

    err = QInfo_create(&QDMI_Parser::info);
    CHECK_ERR(err, "QInfo_create");

    err = QDMI_session_init(QDMI_Parser::info, &QDMI_Parser::session);
    CHECK_ERR(err, "QDMI_session_init");
    if (err != QDMI_SUCCESS)
    {
        std::cout << "   [sys-sage]...............Unable to initiate "
                  << "QDMI session\n";
        exit(0);
    }
    std::cout << "   [sys-sage]...............Initiated "
                  << "QDMI session\n";
    return 0;    

}

// TODO: Change function name (like FOMAC_available_devices)
extern "C" std::vector<std::pair <std::string, QDMI_Device>> QDMI_Parser::get_available_backends()
{
    // TODO REPEAT PERIODICALLY WITH A DELAY

    char **available_devices = get_qdmi_library_list_names();
    if (available_devices == NULL)
    {
        std::cout << "   [sys-sage]...............Failed to get "
                  << "qdmi_library_list from QDMI" << std::endl;

        return std::vector<std::pair <std::string, QDMI_Device>>();
    }

    int i = 0, err = 0, status = 0;
    std::vector<std::pair <std::string, QDMI_Device>> registered_devices;
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

            registered_devices.push_back(std::pair<std::string, QDMI_Device> (device_name, device));
        }

        free(available_devices[i++]);
    }
    free(available_devices);

    return registered_devices;
}

extern "C" int QDMI_Parser::get_num_qubits(QDMI_Device dev)
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

extern "C" void QDMI_Parser::set_qubits(QDMI_Device dev, int device_index)
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

extern "C" void QDMI_Parser::getCouplingMapping(QDMI_Device dev, QDMI_Qubit qubit, std::vector<int> &coupling_mapping, int &coupling_map_size)
{

    if (qubit->size_coupling_mapping == 0)
    {
        std::cout << "   [sys-sage]...............No coupling mapping for qubit\n";
        return;
    }

    coupling_map_size = qubit->size_coupling_mapping;
    coupling_mapping.resize(coupling_map_size);

    //for (int j = 0; j < coupling_map_size; j++)
    //{
        // Copy the coupling_mapping from qubit_impl
        std::copy(qubit->coupling_mapping, qubit->coupling_mapping + coupling_map_size, coupling_mapping.begin());
    //}
}

extern "C" void QDMI_Parser::getQubitProperties(QDMI_Device dev, QDMI_Qubit qubit)
{
    int scope;
    // Declare prop as a vector
    std::vector<int> prop{QDMI_T1_TIME, QDMI_T2_TIME, QDMI_READOUT_ERROR, QDMI_READOUT_LENGTH};
    std::array<std::string, 4> properties{"T1", "T2", "readout_error", "readout_length"};
    double value;
    for (size_t i = 0; i < 4; ++i)
    {
        // QDMI_Qubit_property prop_index;
        QDMI_Qubit_property prop_index = new (QDMI_Qubit_property_impl_t);
        prop_index->name = prop[i];
        int err = QDMI_query_qubit_property_exists(dev, prop_index, qubit, &scope);
        if(err)
        {
            std::cout << "   [sys-sage]...............Queried property doesn't exist: " << i <<"\n";
            continue;
        }
        if(prop_index->type == QDMI_DOUBLE){
            err = QDMI_query_qubit_property_d(dev, qubit, prop_index, &value);
            if(err)
            {
                std::cout << "   [sys-sage]...............Unable to query property: " << i <<"\n";
                continue;
            }
            //std::cout << "   [sys-sage]...............Value of " << properties[i] << ": " << value << "\n";
        }
        delete prop_index;

    }
    
}

extern "C" void QDMI_Parser::setQubits(QuantumBackend *backend, QDMI_Device dev)
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

    num_qubits = backend->GetNumberofQubits();
    
    for (int i = 0; i < num_qubits; i++)
    {
        Qubit* q = new Qubit(backend, i);
        
        // Set coupling map
        int coupling_map_size; 
        std::vector<int> coupling_mapping;
        getCouplingMapping(dev, &qubits[i], coupling_mapping, coupling_map_size);
        q->SetCouplingMapping(coupling_mapping, coupling_map_size);

        // Set all the qubit properties
        getQubitProperties(dev, &qubits[i]);
        q->SetProperties(qubits[i].t1, qubits[i].t2, qubits[i].readout_error, qubits[i].readout_length);
    }

    free(qubits);
    return;
}

extern "C" void QDMI_Parser::setGateSets(QuantumBackend *backend, QDMI_Device dev)
{
    QDMI_Gate gates;
    int err, num_gates;
    
    err = QDMI_query_all_gates(dev, &gates);

    if (err != QDMI_SUCCESS || gates == NULL)
    {
        std::cout << "   [sys-sage]...............Could not obtain available "
                  << "gates via QDMI\n";
        return;
    }

    err = QDMI_query_gateset_num(dev, &num_gates);

    if (err != QDMI_SUCCESS || num_gates == 0)
    {
        std::cout << "   [sys-sage]...............Could not obtain number of "
                  << "available gates via QDMI\n";
        return;
    }
    else 
    {
        std::cout << "   [sys-sage]...............Found "
                  << num_gates << " supported gates.\n";
    }
    //std::vector <std::string> gatesets(num_gates);

    for (int i = 0; i < num_gates; i++)
    {

        std::string name = gates[i].name;
        double fidelity = gates[i].fidelity;
        std::string unitary = gates[i].unitary;
        size_t gate_size = gates[i].gate_size;

        // Add a destructor
        QuantumGate *qgate = new QuantumGate(gate_size);
        qgate->SetGateProperties(name, fidelity, unitary);

        //qgate->setAdditionalProperties();
        backend->addGate(qgate);
    }
    
    //backend->SetGateTypes(gatesets, num_gates);
    
    // for(int num = 0; num < num_gates; ++num)
    // {
    //     auto mapping = gates[num].coupling_mapping;
    //     auto size_coupling_map = gates[num].sizcde_coupling_map;
    //     auto gate_size = gates[num].gate_size;
    //     if(mapping != NULL)
    //     {

    //         // std::cout << "[sys-sage] Coupling map of gate: " << gates[num].name <<", size_coupling_map: "<< size_coupling_map << ", gate_size: " << gate_size<< "\n";
            
    //         for (size_t i = 0; i < size_coupling_map; ++i) {
    //             std::cout << "[";
    //             for (size_t j = 0; j < gate_size; ++j)
    //             {
    //                 std::cout << mapping[i][j] << ", ";
    //             }
    
    //             printf("]\n");
    //         }
    //     }
        
    // }
    
    free(gates);

    return;
}

extern "C" void QDMI_Parser::createQcTopo(Topology *topo)
{
    auto quantum_backends = get_available_backends();


    int total_quantum_backends = quantum_backends.size();

    for (auto i = 0; i < total_quantum_backends; ++i)
    {
        QuantumBackend* qc = new QuantumBackend(topo, i, quantum_backends[i].first);
        createQcTopo(qc, quantum_backends[i].second);
    }

}

extern "C" void QDMI_Parser::createQcTopo(QuantumBackend *backend, QDMI_Device dev)
{
    backend->SetNumberofQubits(get_num_qubits(dev));
    setQubits(backend, dev);
    setGateSets(backend, dev);
}

// extern "C" static void refreshQubitProprties(QuantumBackend *qc, Qubit *qubit)
// {
//     //auto quantum_backends = get_available_backends();

//     // Search for required QuantumBackend in the device and call these:
//     //QDMI_Device dev = qc->
//     //QDMI_query_qubit_property(QDMI_Device dev, QDMI_Qubit_property prop, QDMI_Qubit qubit, int* coupling_map);
// }

void QuantumBackend::RefreshTopology()
{
    
    for (auto child : *(this->GetChildren())) {
        if (auto qubit = dynamic_cast<Qubit*>(child)) {
            qubit->RefreshProperties();
        }
    }
}

void Qubit::RefreshProperties()
{
    QuantumBackend *qc = dynamic_cast<QuantumBackend*> (this->GetParent());
    QDMI_Parser::refreshQubitProprties(qc, this);
    
}
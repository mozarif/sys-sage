/**
* @file qdmi-parser.hpp
* @brief sys-sage's interface to QDMI.
*/

#ifndef QDMIParser_HPP
#define QDMIParser_HPP

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

    class QdmiParser
    {
    public:
        QdmiParser();

        /**
         * Returns all the available backends registered with QDMI.
         * @return A vector of all the backends with their names and "QDMI_Device" handles
         */
        std::vector<std::pair <std::string, QDMI_Device>> get_available_backends();

        int get_num_qubits(QDMI_Device dev);
        /**
         * Checks which backends are available and creates a topology with an object of type "Topology" as their parent.
         * @param topo - Object of type "Topology", which will be the parent Component of all the quntum backends. 
         */
        void createAllQcTopo(Topology *topo);

        /**
         * Checks which backends are available and creates a topology with an object of type "Topology" as their parent.
         * @returns An object of type "Topology", which will be the parent Component of all the quntum backends. 
         * @see createAllQcTopo(Topology *topo)
         */
        Topology createAllQcTopo();

        /**
         * Creates a topology of a single quantum backend corresponding to the provided "QDMI_Device".
         * @param backend - Object of type "QuantumBackend", storing the topology of a quantum backend.
         * @param dev - A QDMI_Device for which the topology needs to be created.
         * @see createQcTopo(Topology *topo)
         */
        void createQcTopo(QuantumBackend *backend, QDMI_Device dev);

        /**
         * Creates a topology of a single quantum backend corresponding to the provided "QDMI_Device".
         * @param dev - A QDMI_Device for which the topology needs to be created.
         * @param device_index - Index of the device.
         * @param device_name - Name of the device.
         * @returns Object of type "QuantumBackend", storing the topology of a quantum backend.
         * @see createQcTopo(QuantumBackend *backend, QDMI_Device dev)
         */
        QuantumBackend createQcTopo(QDMI_Device dev, int device_index = 0, std::string device_name="");

        static void refreshQcProperties(QuantumBackend *qc, bool single_qubit = false, int qubit_index=0)
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

    };
    
}

#endif // QDMIParser_HPP
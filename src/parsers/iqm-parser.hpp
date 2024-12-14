/**
* @file iqm-parser.hpp
* @brief sys-sage's interface to IQM Q-Systems.
*/

#ifndef IQM_PARSER_HPP
#define IQM_PARSER_HPP

#include "Component.hpp"
#include <fstream>
#include <nlohmann/json.hpp>


using json = nlohmann::json;

class IQMParser
{
public: 

    IQMParser(std::ifstream& filepath);

    /**
     * Creates a topology of a single quantum backend corresponding to the provided "QDMI_Device".
     * @param backend - Object of type "QuantumBackend", storing the topology of a quantum backend.
     * @param dev - A QDMI_Device for which the topology needs to be created.
     * @see createQcTopo(Topology *topo)
     */
    void createQcTopo(QuantumBackend *backend);

    /**
     * Creates a topology of a single quantum backend corresponding to the provided "QDMI_Device".
     * @param device_index - Index of the device.
     * @param device_name - Name of the device.
     * @returns Object of type "QuantumBackend", storing the topology of a quantum backend.
     * @see createQcTopo(QuantumBackend *backend, QDMI_Device dev)
     */
    QuantumBackend createQcTopo(int device_index, std::string device_name="");

    QuantumBackend createQcTopo();

private:
    json _data;
    void setQubits(QuantumBackend *backend);


};

#endif // IQM_PARSER_HPP
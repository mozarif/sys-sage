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

int parseIQM(Component* parent, string dataSourcePath, int qcId);
int parseIQM(QuantumBackend* parent, string dataSourcePath, int qcId);

class IQMParser
{
public: 

    IQMParser(QuantumBackend* _qc,std::string filepath);
    json jsonData;
    QuantumBackend * backend;

    /**
     * Creates a topology of a single quantum backend corresponding to the provided "QDMI_Device".
     * @param backend - Object of type "QuantumBackend", storing the topology of a quantum backend.
     * @param dev - A QDMI_Device for which the topology needs to be created.
     * @see createQcTopo(Topology *topo)
     */
    int CreateQcTopo();
    int ParseDynamicData();

    // /**
    //  * Creates a topology of a single quantum backend corresponding to the provided "QDMI_Device".
    //  * @param device_index - Index of the device.
    //  * @param device_name - Name of the device.
    //  * @returns Object of type "QuantumBackend", storing the topology of a quantum backend.
    //  * @see createQcTopo(QuantumBackend *backend, QDMI_Device dev)
    //  */
    // QuantumBackend CreateQcTopo(int device_index, std::string device_name="");

    // QuantumBackend CreateQcTopo();
    
private:
    
    // void setQubits(QuantumBackend *backend);

};

#endif // IQM_PARSER_HPP
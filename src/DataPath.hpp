#ifndef RELATION_HPP
#define RELATION_HPP

#include <map>
#include <vector>
#include <string>

#include "defines.hpp"
#include "Topology.hpp"

#define SYS_SAGE_DATAPATH_NONE 1 /**< TODO */
#define SYS_SAGE_DATAPATH_OUTGOING 2 /**< This Component is the source DataPath. */
#define SYS_SAGE_DATAPATH_INCOMING 4 /**< This Component is the target DataPath. */

//int oriented
#define SYS_SAGE_DATAPATH_BIDIRECTIONAL 8 /**< DataPath has no direction. */
#define SYS_SAGE_DATAPATH_ORIENTED 16 /**< DataPath is directed from the source to the target. */

//datapath type
#define SYS_SAGE_DATAPATH_TYPE_NONE 32 /**< Generic type of DataPath */
#define SYS_SAGE_DATAPATH_TYPE_LOGICAL 64 /**< DataPath describes a logical connection/relation of two Components. */
#define SYS_SAGE_DATAPATH_TYPE_PHYSICAL 128 /**< DataPath describes a physical/hardware connection/relation of two Components. */
#define SYS_SAGE_DATAPATH_TYPE_L3CAT 256 /**< DataPath type describing Cache partitioning settings. */
#define SYS_SAGE_DATAPATH_TYPE_MIG 512 /**< DataPath type describing GPU partitioning settings. */
#define SYS_SAGE_DATAPATH_TYPE_DATATRANSFER 1024 /**< DataPath type describing data transfer attributes. */
#define SYS_SAGE_DATAPATH_TYPE_C2C 2048 /**< DataPath type describing cache-to-cache latencies (cccbench data source). */

#define SYS_SAGE_1Q_QUANTUM_GATE 1 /**< Quantum Gate of size 1-Qubit. */
#define SYS_SAGE_2Q_QUANTUM_GATE 2 /**< Quantum Gate of size 2-Qubits. */
#define SYS_SAGE_MQ_QUANTUM_GATE 4 /**< Quantum Gate of size M-Qubits (where M >2). */
#define SYS_SAGE_NO_TYPE_QUANTUM_GATE 0 /**< Quantum Gate of size 0 or invalid size. */


// QuantumGate type
#define SYS_SAGE_QUANTUMGATE_TYPE_ID 32          /**< Identity Gate */
#define SYS_SAGE_QUANTUMGATE_TYPE_RZ 64          /**< RZ Gate */
#define SYS_SAGE_QUANTUMGATE_TYPE_CNOT 128       /**< CNOT Gate */
#define SYS_SAGE_QUANTUMGATE_TYPE_SX 256         /**< SX Gate */
#define SYS_SAGE_QUANTUMGATE_TYPE_X 512          /**< X Gate */
#define SYS_SAGE_QUANTUMGATE_TYPE_TOFFOLI 1024   /**< Toffoli Gate */
#define SYS_SAGE_QUANTUMGATE_TYPE_UNKNOWN 2048   /**< Unknown Gate */
class Component;
class Qubit;

/**
 * Abstract class Relation representing a generic relationship or connectivity between two components.
 */
class Relation {
public:

    void SetType();
    int GetType();
    void SetId();
    int GetId();
    void SetName();
    std::string GetName();
    int id;
    int type;
    std::string name;

    virtual ~Relation() = default;

    virtual void Print() = 0;

    virtual void DeleteRelation() = 0;
};
class DataPath;
/**
Obsolete; use DataPath() constructors directly instead
@see DataPath()
*/
DataPath* NewDataPath(Component* _source, Component* _target, int _oriented, int _type = SYS_SAGE_DATAPATH_TYPE_NONE);
/**
Obsolete; use DataPath() constructors directly instead
@see DataPath()
*/
DataPath* NewDataPath(Component* _source, Component* _target, int _oriented, double _bw, double _latency);
/**
Obsolete; use DataPath() constructors directly instead
@see DataPath()
*/
DataPath* NewDataPath(Component* _source, Component* _target, int _oriented, int _type, double _bw, double _latency);

/**
Class DataPath represents Data Paths in the topology -- Data Paths represent an arbitrary relation (or data movement) between two Components from the Component Tree.
\n Data Paths create a Data-Path graph, which is a structure orthogonal to the Component Tree.
\n Each Component contains a reference to all Data Paths going to or from this components (as well as parents and children in the Component Tree). Using these references, it is possible to navigate between the Components and to view the data stores in the Components or the Data Paths.
*/

class DataPath : public Relation {

public:
    /**
    DataPath constructor.
    @param _source - pointer to the source Component. (If _oriented == SYS_SAGE_DATAPATH_BIDIRECTIONAL, there is no difference between _source and _target)
    @param _target - pointer to the target Component.
    @param _oriented - is the Data Path oriented? (SYS_SAGE_DATAPATH_ORIENTED = oriented DP; SYS_SAGE_DATAPATH_BIDIRECTIONAL = NOT oriented DP)
    @param _type - (optional parameter) Denotes type of the Data Path -- helps to distinguish Data Paths carrying different type of information. The value is user-defined. If not specified, the Data Path will have type SYS_SAGE_DATAPATH_TYPE_NONE.
        \n Predefined types: SYS_SAGE_DATAPATH_TYPE_NONE, SYS_SAGE_DATAPATH_TYPE_LOGICAL, SYS_SAGE_DATAPATH_TYPE_PHYSICAL, SYS_SAGE_DATAPATH_TYPE_L3CAT. Each user can define an arbitrary type as an integer value > 1024
    */
    DataPath(Component* _source, Component* _target, int _oriented, int _type = SYS_SAGE_DATAPATH_TYPE_NONE);
    /**
    DataPath constructor. DataPath type is set to SYS_SAGE_DATAPATH_TYPE_NONE.
    @param _source - pointer to the source Component. (If _oriented == SYS_SAGE_DATAPATH_BIDIRECTIONAL, there is no difference between _source and _target)
    @param _target - pointer to the target Component.
    @param _oriented - is the Data Path oriented? (SYS_SAGE_DATAPATH_ORIENTED = oriented DP; SYS_SAGE_DATAPATH_BIDIRECTIONAL = NOT oriented DP)
    @param _bw - bandwidth from the source(provides the data) to the target(requests the data)
    @param _latency - Data load latency from the source(provides the data) to the target(requests the data)
    */
    DataPath(Component* _source, Component* _target, int _oriented, double _bw, double _latency);
    /**
    DataPath constructor.
    @param _source - pointer to the source Component. (If _oriented == SYS_SAGE_DATAPATH_BIDIRECTIONAL, there is no difference between _source and _target)
    @param _target - pointer to the target Component.
    @param _oriented - is the Data Path oriented? (SYS_SAGE_DATAPATH_ORIENTED = oriented DP; SYS_SAGE_DATAPATH_BIDIRECTIONAL = NOT oriented DP)
    @param _type - (optional parameter) Denotes type of the Data Path -- helps to distinguish Data Paths carrying different type of information. The value is user-defined. If not specified, the Data Path will have type SYS_SAGE_DATAPATH_TYPE_NONE.
        \n Predefined types: SYS_SAGE_DATAPATH_TYPE_NONE, SYS_SAGE_DATAPATH_TYPE_LOGICAL, SYS_SAGE_DATAPATH_TYPE_PHYSICAL, SYS_SAGE_DATAPATH_TYPE_L3CAT. Each user can define an arbitrary type as an integer value > 1024
    @param _bw - Bandwidth from the source(provides the data) to the target(requests the data)
    @param _latency - Data load latency from the source(provides the data) to the target(requests the data)
    */
    DataPath(Component* _source, Component* _target, int _oriented, int _type, double _bw, double _latency);

    /**
    @returns Pointer to the source Component
    */
    Component* GetSource();
    /**
    @returns Pointer to the target Component
    */
    Component* GetTarget();
    /**
    @returns Bandwidth from the source(provides the data) to the target(requests the data)
    */
    double GetBw();
    /**
    @returns Data load latency from the source(provides the data) to the target(requests the data)
    */
    double GetLatency();
    /**
    @returns Type of the Data Path.
    @see type
    */
    //TO-DO: Replace this with type, no type required
    int GetDpType();
    /**
     * TODO
    */
    int GetOriented();

    /**
    Prints basic information about the Data Path to stdout. Prints componentType and Id of the source and target Components, the bandwidth, load latency, and the attributes; for each attribute, the name and value are printed, however the value is only retyped to uint64_t (therefore will print nonsensical values for other data types).
    */
    void Print();

    /**
    Deletes and de-allocated the DataPath pointer from the list(std::vector) of outgoing and incoming DataPaths of source and target Components.
    @see dp_incoming
    @see dp_outgoing
    */
    void DeleteDataPath();

    void DeleteRelation() override;
    /**
     * TODO
    */
    std::map<std::string,void*> attrib;

private:
    Component *source;
    Component *target;
    const int oriented;
    const int type;
    double bw;
    double latency;
};

class QuantumGate : public Relation {

public:
    QuantumGate();
    QuantumGate(size_t _gate_size);
    QuantumGate(size_t _gate_size, std::string _name, double _fidelity, std::string _unitary);
    QuantumGate(size_t _gate_size, const std::vector<Qubit *> & _qubits);
    QuantumGate(size_t _gate_size, const std::vector<Qubit *> & _qubits, std::string _name, double _fidelity, std::string _unitary);

    void SetGateProperties(std::string _name, double _fidelity, std::string _unitary);
    void SetGateCouplingMap(std::vector<std::vector<Qubit*>> _coupling_mapping);
    void SetAdditionalProperties();
    void SetType();
    int GetType();
    double GetFidelity() const;
    size_t GetGateSize() const;
    void SetId(int _id);
    int GetId();
    std::string GetUnitary() const;
    std::string GetName();
    void Print() override;
    void DeleteRelation() override;

private:

    size_t gate_size;
    int gate_length;
    std::string unitary;
    double fidelity;
    std::vector<std::vector<Qubit*>> coupling_mapping;
    std::vector<Qubit*> qubits;
    std::map<std::string, double> additional_properties;
};

#endif // RELATION_HPP

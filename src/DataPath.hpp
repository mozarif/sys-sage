#ifndef RELATION_HPP
#define RELATION_HPP

#include <map>
#include <vector>
#include <string>

#include "defines.hpp"
// #include "Component.hpp"

// //Component pointing to a DataPath 
// #define SYS_SAGE_DATAPATH_NONE 1 /**< Reserved for development purposes. */
// #define SYS_SAGE_DATAPATH_OUTGOING 2 /**< This Component is the source DataPath. */
// #define SYS_SAGE_DATAPATH_INCOMING 4 /**< This Component is the target DataPath. */

// //int oriented
// #define SYS_SAGE_DATAPATH_BIDIRECTIONAL 8 /**< DataPath has no direction. */
// #define SYS_SAGE_DATAPATH_ORIENTED 16 /**< DataPath is directed from the source to the target. */

// //datapath type
// #define SYS_SAGE_DATAPATH_TYPE_NONE 32 /**< Generic type of DataPath */
// #define SYS_SAGE_DATAPATH_TYPE_LOGICAL 64 /**< DataPath describes a logical connection/relation of two Components. */
// #define SYS_SAGE_DATAPATH_TYPE_PHYSICAL 128 /**< DataPath describes a physical/hardware connection/relation of two Components. */
// #define SYS_SAGE_DATAPATH_TYPE_L3CAT 256 /**< DataPath type describing Cache partitioning settings. */
// #define SYS_SAGE_DATAPATH_TYPE_MIG 512 /**< DataPath type describing GPU partitioning settings. */
// #define SYS_SAGE_DATAPATH_TYPE_DATATRANSFER 1024 /**< DataPath type describing data transfer attributes. */
// #define SYS_SAGE_DATAPATH_TYPE_C2C 2048 /**< DataPath type describing cache-to-cache latencies (cccbench data source). */

// #define SYS_SAGE_1Q_QUANTUM_GATE 1 /**< Quantum Gate of size 1-Qubit. */
// #define SYS_SAGE_2Q_QUANTUM_GATE 2 /**< Quantum Gate of size 2-Qubits. */
// #define SYS_SAGE_MQ_QUANTUM_GATE 4 /**< Quantum Gate of size M-Qubits (where M >2). */
// #define SYS_SAGE_NO_TYPE_QUANTUM_GATE 0 /**< Quantum Gate of size 0 or invalid size. */


// // QuantumGate type
// #define SYS_SAGE_QUANTUMGATE_TYPE_ID 32          /**< Identity Gate */
// #define SYS_SAGE_QUANTUMGATE_TYPE_RZ 64          /**< RZ Gate */
// #define SYS_SAGE_QUANTUMGATE_TYPE_CNOT 128       /**< CNOT Gate */
// #define SYS_SAGE_QUANTUMGATE_TYPE_SX 256         /**< SX Gate */
// #define SYS_SAGE_QUANTUMGATE_TYPE_X 512          /**< X Gate */
// #define SYS_SAGE_QUANTUMGATE_TYPE_TOFFOLI 1024   /**< Toffoli Gate */
// #define SYS_SAGE_QUANTUMGATE_TYPE_UNKNOWN 2048   /**< Unknown Gate */


// #define SYS_SAGE_RELATION_COUPLING_MAP 1001

///////////////////////////////////////////////////

namespace sys_sage { //forward declaration
    class Component;
    class Qubit;
}

namespace sys_sage {
    namespace RelationType{
        using type = int32_t;

        constexpr type Any = -1;
        constexpr type Relation = 0;
        constexpr type DataPath = 1;
        constexpr type QuantumGate = 2;
        constexpr type CouplingMap = 3;
        constexpr type _num_relation_types = 4;

        constexpr type RelationTypeList [_num_relation_types] = {
            Relation, 
            DataPath, 
            QuantumGate, 
            CouplingMap
        };

        //SVTODO this should remain private???
        static const std::unordered_map<type, const char*> names = {
            {Any, "Any"},
            {Relation, "Relation"},
            {DataPath, "DataPath"},
            {QuantumGate, "QuantumGate"},
            {CouplingMap, "CouplingMap"}
        };

        inline const char* ToString(type rt) {
            auto it = names.find(rt);
            if (it != names.end()) return it->second;
            return "Unknown";
        }
    }

    namespace DataPathType{
        using type = int32_t;

        constexpr type Any = 1;
        constexpr type None = 0; /**< Generic type of DataPath */
        constexpr type Logical = 1; /**< DataPath describes a logical connection/relation of two Components. */
        constexpr type Physical = 2; /**< DataPath describes a physical/hardware connection/relation of two Components. */
        constexpr type Datatransfer = 3; /**< DataPath type describing data transfer attributes. */
        constexpr type L3CAT = 4; /**< DataPath type describing Cache partitioning settings. */
        constexpr type MIG = 5; /**< DataPath type describing GPU partitioning settings. */
        constexpr type C2C = 6; /**< DataPath type describing cache-to-cache latencies (cccbench data source). */
    }
    namespace DataPathDirection{
        using type = int32_t;

        constexpr type Any = 1;
        constexpr type Outgoing = 2; /**< This Component is the source DataPath. */
        constexpr type Incoming = 3; /**< This Component is the target DataPath. */
    }

    namespace DataPathOrientation{
        using type = int32_t;

        constexpr type Oriented = 1; /**< DataPath is directed from the source to the target. */
        constexpr type NotOriented = 2; /**< DataPath has no direction. */
    }
    namespace QuantumGateType{
        using type = int32_t;

        constexpr type Unknown = 0; /**< Unknown Gate */
        constexpr type Id = 1; /**< Identity Gate */
        constexpr type X = 2; /**< X (Not) Gate */
        constexpr type Rz = 3; /**< RZ Gate */
        constexpr type Cnot = 4; /**< CNOT Gate */
        constexpr type Sx = 5; /**< SX Gate */
        constexpr type Toffoli = 6; /**< Toffoli Gate */
    }




    //SVTODO make all non-public (non-API) methods with _prefix

    /**
     * @class Relation
     * @brief Represents a generic relationship or connectivity between an arbitrary number of components.
     *
     * This class defines a generic relationship with methods to set and get
     * the type, id, and name of the relationship. It also provides pure virtual
     * functions for printing and deleting the relationship, making it an abstract class.
     */
    class Relation {
    public:
        Relation(std::initializer_list<Component*> components, bool _ordered = true);
        /**
         * @brief Sets the id of the relationship.
         * @param _id The id of the relationship to set.
         */
        void SetId(int _id);
        /**
         * @brief Gets the id of the relationship.
         * @return The current id of the relationship.
         */
        int GetId();
        /**
         * @brief Gets the type of the relationship.
         * @return The current type of the relationship.
         */
        int GetType();
        //SVDOCTODO
        std::string GetTypeStr();
        //SVDOCTODO
        bool IsOrdered();
        //SVDOCTODO
        bool ContainsComponent(Component* c);
        //SVDOCTODO
        Component* GetComponent(int index);
        //SVDOCTODO
        const std::vector<Component*>& GetComponents() const;

        //SVDOCTODO
        /**
         * @brief Pure virtual function to print the details of the relationship.
         * 
         * Derived classes must implement this function to provide specific
         * printing behavior.
         */
        virtual void Print();
        //SVDOCTODO
        void AddComponent(Component* c);
        //SVDOCTODO
        void UpdateComponent(int index, Component * _new_component);
        //SVDOCTODO
        //SVDOCTODO mention that it only replaces first entry of _old_component found (using std::find)
        void UpdateComponent(Component* _old_component, Component * _new_component);

        //SVDOCTODO
        /**
         * @brief Pure virtual function to delete the relationship.
         * 
         * Derived classes must implement this function to provide specific
         * deletion behavior.
         */
        virtual void Delete();//TODO
        /**
         * @brief Destructor for the Relation class.
         * 
         * This is a virtual destructor to ensure proper cleanup of derived classes.
         */
        ~Relation() = default;
    protected:
        Relation(int _relation_type);

        bool ordered;
        /**
         * @brief The id of the relationship.
         * 
         * This member variable stores the unique identifier for the relationship.
         */
        int id;
        /**
         * @brief The type of the relationship.
         * 
         * This member variable stores the type or category of the relationship.
         */
        int type;
        /**
         * @brief A vector of components associated with the relationship.
         * 
         * This member variable holds pointers to components that are part of
         * the relationship.
         */
        std::vector<Component*> components;

    public:
        /**
        * A map for storing arbitrary pieces of information or data.
        * - The `key` denotes the name of the attribute.
        * - The `value` points to the data, stored as a `void*`.
        *
        * This data structure is designed to store a wide variety of data types by
        * utilizing pointers to void. Due to its flexibility, it is essential to manage
        * the types and memory allocation/deallocation carefully to avoid issues such
        * as memory leaks or undefined behavior.
        *
        * Usage:
        * 
        * 1. Adding a new key-value pair:
        * 
        * ```cpp
        * std::string key = "exampleKey";
        * int* value = new int(42); // Dynamically allocate memory for the value
        * attrib[key] = static_cast<void*>(value); // Store the value in the map
        * ```
        * 
        * 2. Retrieving data from an existing key:
        * 
        * ```cpp
        * std::string key = "exampleKey";
        * if (attrib.find(key) != attrib.end()) {
        *     int* retrievedValue = static_cast<int*>(attrib[key]);
        *     std::cout << "Value: " << *retrievedValue << std::endl;
        * } else {
        *     std::cout << "Key not found." << std::endl;
        * }
        * ```
        * 
        * 3. Checking for the existence of a key:
        * 
        * ```cpp
        * std::string key = "exampleKey";
        * if (attrib.find(key) != attrib.end()) {
        *     std::cout << "Key exists." << std::endl;
        * } else {
        *     std::cout << "Key does not exist." << std::endl;
        * }
        * ```
        * 
        * 4. Removing a key-value pair and freeing memory:
        * 
        * ```cpp
        * std::string key = "exampleKey";
        * if (attrib.find(key) != attrib.end()) {
        *     int* value = static_cast<int*>(attrib[key]);
        *     delete value; // Free the dynamically allocated memory
        *     attrib.erase(key); // Remove the key-value pair from the map
        * }
        * ```
        * 
        * 5. Updating the value for an existing key:
        * 
        * ```cpp
        * std::string key = "exampleKey";
        * if (attrib.find(key) != attrib.end()) {
        *     int* oldValue = static_cast<int*>(attrib[key]);
        *     delete oldValue; // Free the old value
        *     int* newValue = new int(100); // Allocate new value
        *     attrib[key] = static_cast<void*>(newValue); // Update the map
        * }
        * ```
        * 
        * Note:
        * - Proper memory management is crucial when using `void*` pointers. Always ensure
        *   that dynamically allocated memory is freed when no longer needed.
        * - Type safety is not enforced, so it is important to cast pointers to the correct
        *   type when retrieving values from the map.
        */
        std::map<std::string, void*> attrib;
    };


    /**
    Class DataPath represents Data Paths in the topology -- Data Paths represent an arbitrary relation (or data movement) between two Components from the Component Tree.
    \n Data Paths create a Data-Path graph, which is a structure orthogonal to the Component Tree.
    \n Each Component contains a reference to all Data Paths going to or from this components (as well as parents and children in the Component Tree). Using these references, it is possible to navigate between the Components and to view the data stores in the Components or the Data Paths.
    */

    class DataPath : public Relation {

    public:
        //SVDOCTODO
        /**
        DataPath constructor.
        @param _source - pointer to the source Component. (If _oriented == SYS_SAGE_DATAPATH_BIDIRECTIONAL, there is no difference between _source and _target)
        @param _target - pointer to the target Component.
        @param _oriented - is the Data Path oriented? (SYS_SAGE_DATAPATH_ORIENTED = oriented DP; SYS_SAGE_DATAPATH_BIDIRECTIONAL = NOT oriented DP)
        @param _type - (optional parameter) Denotes type of the Data Path -- helps to distinguish Data Paths carrying different type of information. The value is user-defined. If not specified, the Data Path will have type SYS_SAGE_DATAPATH_TYPE_NONE.
            \n Predefined types: SYS_SAGE_DATAPATH_TYPE_NONE, SYS_SAGE_DATAPATH_TYPE_LOGICAL, SYS_SAGE_DATAPATH_TYPE_PHYSICAL, SYS_SAGE_DATAPATH_TYPE_L3CAT. Each user can define an arbitrary type as an integer value > 1024
        */
        DataPath(Component* _source, Component* _target, sys_sage::DataPathOrientation::type _oriented, sys_sage::DataPathType::type _dp_type = sys_sage::DataPathType::None);
        //SVDOCTODO
        /**
        DataPath constructor. DataPath type is set to SYS_SAGE_DATAPATH_TYPE_NONE.
        @param _source - pointer to the source Component. (If _oriented == SYS_SAGE_DATAPATH_BIDIRECTIONAL, there is no difference between _source and _target)
        @param _target - pointer to the target Component.
        @param _oriented - is the Data Path oriented? (SYS_SAGE_DATAPATH_ORIENTED = oriented DP; SYS_SAGE_DATAPATH_BIDIRECTIONAL = NOT oriented DP)
        @param _bw - bandwidth from the source(provides the data) to the target(requests the data)
        @param _latency - Data load latency from the source(provides the data) to the target(requests the data)
        */
        DataPath(Component* _source, Component* _target, sys_sage::DataPathOrientation::type _oriented, double _bw, double _latency);
        //SVDOCTODO
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
        DataPath(Component* _source, Component* _target, sys_sage::DataPathOrientation::type _oriented, sys_sage::DataPathType::type _dp_type, double _bw, double _latency);

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
        double GetBandwidth();
        
        /**
         * Sets the bandwidth from the source to the target.
        @param _bandwidth - value for bw
        @see bw
        */
        void SetBandwidth(double _bandwidth);
        /**
        @returns Data load latency from the source(provides the data) to the target(requests the data)
        */
        double GetLatency();

        /**
         * Sets the data load latency from the source to the target.
        @param _latency - value for latency
        @see latency
        */
        void SetLatency(double _latency);
        
        /**
         * Updates the source of the DataPath. The datapath is removed from the outgoing/incoming vectors of the old source and 
         * added to the vectors of the newer source.
         * @param _new_source - New source of the DataPath.
         */
        void UpdateSource(Component * _new_source);

        /**
         * Updates the target of the DataPath. The datapath is removed from the outgoing/incoming vectors of the old target and 
         * added to the vectors of the newer target.
         * @param _new_source - New target of the DataPath.
         */
        void UpdateTarget(Component * _new_target);
        
        /**
         * @private
        Obsolete;use GetType() instead.
        @returns Type of the Data Path.
        @see type
        */
        int GetDataPathType();
        //SVDOCTODO mention ordered instead of orientation
        /**
         * Retrieves the orientation of the datapath (SYS_SAGE_DATAPATH_ORIENTED or SYS_SAGE_DATAPATH_BIDIRECTIONAL)
         * @returns orientation 
         * @see oriented
         * 
        */
        int GetOrientation();

        /**
        Prints basic information about the Data Path to stdout. Prints componentType and Id of the source and target Components, the bandwidth, load latency, and the attributes; for each attribute, the name and value are printed, however the value is only retyped to uint64_t (therefore will print nonsensical values for other data types).
        */
        void Print() override;

        //SVDOCTODO update doc
        /**
        Deletes and de-allocates the DataPath pointer from the list(std::vector) of outgoing and incoming DataPaths of source and target Components.
        @see dp_incoming
        @see dp_outgoing
        */
        void Delete() override;

    private:
        // Component * source; /**< source component of the datapath */
        // Component * target; /**< taget component of the datapath */

        // int oriented; /**< orientation of the datapath (SYS_SAGE_DATAPATH_ORIENTED or SYS_SAGE_DATAPATH_BIDIRECTIONAL) */
        int dp_type; /**< type of the datapath */

        double bw; /**< Bandwidth from the source(provides the data) to the target(requests the data) */
        double latency; /**< Data load latency from the source(provides the data) to the target(requests the data) */

    };

    /**
     * @class QuantumGate
     * @brief Represents a quantum gate in a quantum circuit.
     *
     * This class extends the Relation class to model quantum gates. 
     * It includes various properties specific to quantum gates, such as gate size, fidelity, and unitary matrix.
     * The class also handles the coupling map and additional properties related to quantum gate operations.
     */
    class QuantumGate : public Relation {

    public:
        /**
         * @brief Default constructor for QuantumGate.
         * 
         * Initializes a quantum gate with default properties.
         */
        QuantumGate();

        /**
         * @brief Constructor that initializes a quantum gate with a specified size.
         * @param _gate_size The number of qubits this gate operates on.
         */
        QuantumGate(size_t _gate_size);

        /**
         * @brief Constructor that initializes a quantum gate with specific properties.
         * @param _gate_size The number of qubits this gate operates on.
         * @param _name The name of the quantum gate.
         * @param _fidelity The fidelity of the quantum gate.
         * @param _unitary The unitary matrix representing the quantum gate operation.
         */
        QuantumGate(size_t _gate_size, std::string _name, double _fidelity, std::string _unitary);

        /**
         * @brief Constructor that initializes a quantum gate with a list of qubits.
         * @param _gate_size The number of qubits this gate operates on.
         * @param _qubits A vector of pointers to Qubit objects that the gate operates on.
         */
        QuantumGate(size_t _gate_size, const std::vector<Qubit *> & _qubits);

        /**
         * @brief Constructor that initializes a quantum gate with detailed properties and qubits.
         * @param _gate_size The number of qubits this gate operates on.
         * @param _qubits A vector of pointers to Qubit objects that the gate operates on.
         * @param _name The name of the quantum gate.
         * @param _fidelity The fidelity of the quantum gate.
         * @param _unitary The unitary matrix representing the quantum gate operation.
         */
        QuantumGate(size_t _gate_size, const std::vector<Qubit *> & _qubits, std::string _name, double _fidelity, std::string _unitary);

        /**
         * @brief Sets the properties of the quantum gate.
         * @param _name The name of the quantum gate.
         * @param _fidelity The fidelity of the quantum gate.
         * @param _unitary The unitary matrix representing the quantum gate operation.
         */
        void SetGateProperties(std::string _name, double _fidelity, std::string _unitary);

        /**
         * @brief Sets the coupling map for the quantum gate.
         * @param _coupling_mapping A vector of vectors of pointers to Qubit objects representing the coupling map.
         */
        void SetGateCouplingMap(std::vector<std::vector<Qubit*>> _coupling_mapping);

        /**
         * @brief Sets additional properties for the quantum gate.
         * 
         * This function allows setting properties that are not part of the core quantum gate attributes.
         */
        void SetAdditionalProperties();

        /**
         * @brief Sets the type of the quantum gate.
         * 
         * Sets the specific type for quantum gates.
         */
        void SetQuantumGateType();

        /**
         * @brief Gets the type of the quantum gate.
         * @return The type identifier for the quantum gate.
         */
        sys_sage::QuantumGateType::type GetQuantumGateType();

        /**
         * @brief Gets the fidelity of the quantum gate.
         * @return The fidelity of the quantum gate.
         */
        double GetFidelity() const;

        /**
         * @brief Gets the size of the quantum gate.
         * @return The number of qubits this gate operates on.
         */
        size_t GetGateSize() const;

        /**
         * @brief Sets the id of the quantum gate.
         * @param _id The id to assign to the quantum gate.
         */
        void SetId(int _id);

        // /**
        //  * @brief Gets the id of the quantum gate.
        //  * @return The id of the quantum gate.
        //  */
        // int GetId();

        /**
         * @brief Gets the unitary matrix of the quantum gate.
         * @return A string representing the unitary matrix of the quantum gate.
         */
        std::string GetUnitary() const;

        /**
         * @brief Sets the name of the relationship.
         * @param _name The name of the relationship to set.
         */
        void SetName(std::string _name);

        /**
         * @brief Gets the name of the relationship.
         * @return The current name of the relationship.
         */
        std::string GetName();

        /**
         * @brief Prints the details of the quantum gate.
         * 
         * This method overrides the Print function in the Relation class to provide specific printing behavior for quantum gates.
         */
        void Print() override;

        // /**
        //  * @brief Deletes the quantum gate relation.
        //  * 
        //  * This method overrides the DeleteRelation function in the Relation class to handle specific deletion behavior for quantum gates.
        //  */
        // void DeleteRelation();

    private:

        /**
         * @brief The size of the quantum gate, representing the number of qubits it operates on.
         */
        size_t gate_size;

        /**
         * @brief The name of the relationship.
         * 
         * This member variable stores the name or description of the relationship.
         */
        std::string name;

        // /**
        //  * @brief List of the qubits involved in a quantum gate. 
        //  * 
        //  * This vector of qubit pointers stores the list of the qubits involved in the gate.
        //  */
        // std::vector<Qubit*> qubits;

        /**
         * @brief The length of the quantum gate operation in terms of time or depth.
         */
        int gate_length;

        sys_sage::QuantumGateType::type gate_type;
        /**
         * @brief The fidelity of the quantum gate, indicating its accuracy or performance.
         */
        double fidelity;

        /**
         * @brief The unitary matrix representing the quantum gate's operation.
         * 
         * This string stores a serialized or symbolic representation of the unitary matrix.
         */
        std::string unitary;

        // /**
        //  * @brief The coupling map for the quantum gate.
        //  * 
        //  * This 2D vector of qubit pointers represents the coupling map, defining how qubits interact within the gate.
        //  */
        // std::vector<std::vector<Qubit*>> coupling_mapping;


        // /**
        //  * @brief Additional properties of the quantum gate.
        //  * 
        //  * This map allows storing additional key-value pairs representing various other properties related to the quantum gate.
        //  */
        // std::map<std::string, double> additional_properties;
    };

    class CouplingMap : public Relation {
    public:
        CouplingMap(Qubit* q1, Qubit*q2);
        // CouplingMap();

        // void Print();
        // void DeleteRelation();
        
        void SetFidelity(double _fidelity);
        double GetFidelity();
        void Delete() override;
    private:
        double fidelity;
    };
} //namespace sys_sage 
#endif // RELATION_HPP

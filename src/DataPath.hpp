#ifndef DATAPATH_CPP
#define DATAPATH_CPP

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

#include "Relation.hpp"

namespace sys_sage { //forward declaration
    class Component;
}

namespace sys_sage {




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
        Component* GetSource() const;
        /**
        @returns Pointer to the target Component
        */
        Component* GetTarget() const;
        /**
        @returns Bandwidth from the source(provides the data) to the target(requests the data)
        */
        double GetBandwidth() const;
        /**
         * Sets the bandwidth from the source to the target.
        @param _bandwidth - value for bw
        @see bw
        */
        void SetBandwidth(double _bandwidth);
        /**
        @returns Data load latency from the source(provides the data) to the target(requests the data)
        */
        double GetLatency() const;
        /**
         * Sets the data load latency from the source to the target.
        @param _latency - value for latency
        @see latency
        */
        void SetLatency(double _latency);

        //SVDOCTODO
        /**
         * Updates the source of the DataPath. The datapath is removed from the outgoing/incoming vectors of the old source and 
         * added to the vectors of the newer source.
         * @param _new_source - New source of the DataPath.
         */
        int UpdateSource(Component * _new_source);

        //SVDOCTODO
        /**
         * Updates the target of the DataPath. The datapath is removed from the outgoing/incoming vectors of the old target and 
         * added to the vectors of the newer target.
         * @param _new_source - New target of the DataPath.
         */
        int UpdateTarget(Component * _new_target);
        
        //SVDOCTODO not private/obsolete -> Relation::Type returns RelationType; GetDataPathType returns DataPathType
        /**
         * @private
        Obsolete;use GetType() instead.
        @returns Type of the Data Path.
        @see type
        */
        int GetDataPathType() const;
        //SVDOCTODO mention ordered instead of orientation
        /**
         * Retrieves the orientation of the datapath (SYS_SAGE_DATAPATH_ORIENTED or SYS_SAGE_DATAPATH_BIDIRECTIONAL)
         * @returns orientation 
         * @see oriented
         * 
        */
        int GetOrientation() const;

        /**
        Prints basic information about the Data Path to stdout. Prints componentType and Id of the source and target Components, the bandwidth, load latency, and the attributes; for each attribute, the name and value are printed, however the value is only retyped to uint64_t (therefore will print nonsensical values for other data types).
        */
        void Print() const override;
        //SVDOCTODO private
        xmlNodePtr _CreateXmlEntry() override;
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


 
} //namespace sys_sage 
#endif // RELATION_HPP

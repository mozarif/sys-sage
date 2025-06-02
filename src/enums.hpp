#ifndef ENUMS_HPP
#define ENUMS_HPP

#include <cstdint>
#include <unordered_map>


namespace sys_sage {

/////////////////////////////////////////////////////////////
////////////////////// COMPONENT ENUMS //////////////////////
/////////////////////////////////////////////////////////////

    namespace ComponentType{
        using type = int32_t;

        constexpr type None = 1; /**< class Component (do not use normally)*/
        constexpr type Thread = 2; /**< class Thread */
        constexpr type Core = 3; /**< class Core */
        constexpr type Cache = 4; /**< class Cache */
        constexpr type Subdivision = 5; /**< class Subdivision */
        constexpr type Numa = 6; /**< class Numa */
        constexpr type Chip = 7; /**< class Chip */
        constexpr type Memory = 8; /**< class Memory */
        constexpr type Storage = 9; /**< class Storage */
        constexpr type Node = 10; /**< class Node */
        constexpr type QuantumBackend = 11; /**< class QuantumBackend */
        constexpr type AtomSite = 12; /**< class AtomSite */
        constexpr type Qubit = 13; /**< class Qubit */
        constexpr type Topology = 14; /**< class Topology */

        //SVTODO this should remain private???
        static const std::unordered_map<type, const char*> names = {
            {None, "GenericComponent"},
            {Thread, "HW_Thread"},
            {Core, "Core"},
            {Cache, "Cache"},
            {Subdivision, "Subdivision"},
            {Numa, "NUMA"},
            {Chip, "Chip"},
            {Memory, "Memory"},
            {Storage, "Storage"},
            {Node, "Node"},
            {QuantumBackend, "QuantumBackend"},
            {AtomSite, "AtomSite"},
            {Qubit, "Qubit"},
            {Topology, "Topology"}
        };

        inline const char* ToString(type rt) {
            auto it = names.find(rt);
            if (it != names.end()) return it->second;
            return "Unknown";
        }
    }

    namespace SubdivisionType {
        using type = int32_t;

        constexpr type None = 1; /**< Generic Subdivision type. */
        constexpr type GpuSM = 2; /**< Subdivision type for GPU SMs */
    }

    namespace ChipType {
        using type = int32_t;

        constexpr type None = 1; /**< Generic Chip type. */
        constexpr type Cpu = 2; /**< Chip type used for a CPU. */
        constexpr type CpuSocket = 3; /**< Chip type used for one CPU socket. */
        constexpr type Gpu = 4; /**< Chip type used for a GPU.*/
    }

////////////////////////////////////////////////////////////
////////////////////// RELATION ENUMS //////////////////////
////////////////////////////////////////////////////////////

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

        //SVTODO this should remain private???
        static const std::unordered_map<type, const char*> names = {
            {Unknown, "Unknown"},
            {Id, "Id"},
            {X, "X"},
            {Rz, "Rz"},
            {Cnot, "Cnot"},
            {Sx, "Sx"},
            {Toffoli, "Toffoli"}
        };

        inline const char* ToString(type rt) {
            auto it = names.find(rt);
            if (it != names.end()) return it->second;
            return "Unknown";
        }
    }


}


#endif //ENUMS_HPP
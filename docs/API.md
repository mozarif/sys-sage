# _sys-sage_'s Public API

This part of the documentation provides information about the various classes and routines the library provides as part of the public API accesible to the user.

## Overview

- Component Tree
    - sys_sage::Component
    - sys_sage::Topology
    - sys_sage::Node
    - sys_sage::Storage
    - sys_sage::Memory
    - sys_sage::Chip
    - sys_sage::Subdivision
    - sys_sage::Numa
    - sys_sage::Cache
    - sys_sage::Core
    - sys_sage::Thread
    - sys_sage::QuantumBackend
    - sys_sage::Qubit
    - sys_sage::AtomSite
- Relations Graph
    - sys_sage::Relation
    - sys_sage::DataPath
    - sys_sage::QuantumGate
    - sys_sage::CouplingMap
- Constants
- Input Parsers
    - [hwloc](hwloc_8hpp.html)
    - MT4G
    - IQM
    - caps-numa-benchmark
    - cccbench
- Data Sources
    - [hwloc](hwloc-output_8cpp.html)
    - MT4G
    - caps-numa-benchmark
- Serialization & Deserialization
    - JSON
- 3rd Party Integrations
    - Intel PQoS
    - NVIDIA MIG
    - proc_cpuinfo
    - PAPI
    - QDMI

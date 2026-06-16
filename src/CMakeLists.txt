# header with #cmakedefine macros for build options
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/defines.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/defines.hpp)

######################################################
#################### SOURCE FILES ####################
######################################################

target_sources(sys-sage PRIVATE
    Component.cpp
    Thread.cpp
    Core.cpp
    Cache.cpp
    Subdivision.cpp
    Numa.cpp
    Chip.cpp
    Memory.cpp
    Storage.cpp
    Node.cpp
    QuantumBackend.cpp
    Qubit.cpp
    AtomSite.cpp
    Topology.cpp
    Relation.cpp
    DataPath.cpp
    QuantumGate.cpp
    CouplingMap.cpp
    xml_dump.cpp
    xml_load.cpp
    json_serialization.cpp
    parsers/hwloc.cpp
    parsers/caps-numa-benchmark.cpp
    parsers/mt4g.cpp
    parsers/mt4g_v0_1.cpp
    parsers/cccbench.cpp
    parsers/iqm-parser.cpp
)

if(INTEL_PQOS)
    target_sources(sys-sage PRIVATE external_interfaces/intel_pqos.cpp)
endif()

if(NVIDIA_MIG)
    target_sources(sys-sage PRIVATE external_interfaces/nvidia_mig.cpp)
endif()

if(PROC_CPUINFO)
    target_sources(sys-sage PRIVATE external_interfaces/proc_cpuinfo.cpp)
endif()

if(PAPI)
    target_sources(sys-sage PRIVATE external_interfaces/ss_papi.cpp)
endif()

if(QDMI)
    target_sources(sys-sage PRIVATE parsers/qdmi-parser.cpp)
endif()

######################################################
################ INCLUDES & LINKING ##################
######################################################

include(GNUInstallDirs)

target_include_directories(sys-sage PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(sys-sage PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(sys-sage PUBLIC LibXml2::LibXml2)

if(NVIDIA_MIG)
    target_link_libraries(sys-sage PUBLIC CUDA::nvml)
endif()

if(PAPI)
    target_include_directories(sys-sage PUBLIC ${PAPI_INCLUDE_DIRS})
    target_link_libraries(sys-sage PUBLIC ${PAPI_LIBRARIES})
endif()

if(QDMI)
    target_link_libraries(sys-sage PUBLIC
        backend_ibm
        qdmi
        qinfo
    )
endif()

######################################################
#################### INSTALLATION ####################
######################################################

install(
    TARGETS sys-sage
    EXPORT sys-sage-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    DIRECTORY .
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.inl"
)

install(
    EXPORT sys-sage-targets
    FILE sys-sage-targets.cmake
    NAMESPACE sys-sage::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sys-sage
)

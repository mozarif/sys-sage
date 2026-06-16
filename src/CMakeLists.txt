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
################### PYTHON BINDINGS ##################
######################################################

if(PY_SYS_SAGE)
    # Get Python site-packages path
    execute_process(
        COMMAND "${Python_EXECUTABLE}" -c "import sysconfig; print(sysconfig.get_paths().get('purelib', ''))"
        OUTPUT_VARIABLE PYTHON_SITE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    #set install directory of the python library
    if(CMAKE_INSTALL_PREFIX STREQUAL "")
        # use install prefix if result is empty
        if(PYTHON_SITE STREQUAL "")
            set(PYTHON_SITE "${CMAKE_INSTALL_PREFIX}/lib/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages")
            message(WARNING "PYTHON_SITE was empty, falling back to: ${PYTHON_SITE}")
        # else()
            # message(STATUS "Python site-packages directory: ${PYTHON_SITE}")
        endif()
    else()
        set(PYTHON_SITE "${CMAKE_INSTALL_PREFIX}/lib/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages")
        # message(STATUS "Python site-packages directory: ${PYTHON_SITE}")
    endif()

    # set(PY_BINDS ${CMAKE_SOURCE_DIR}/src/python-bindings)
    message(STATUS "PY_BINDS=${PY_BINDS}")

    # This builds a Python extension module (NOT a regular .so/.dylib)
    pybind11_add_module(sys_sage MODULE python-bindings/sys-sage-bindings.cpp)
    target_link_libraries(sys_sage PRIVATE sys-sage)

    install(
        TARGETS sys_sage
        LIBRARY DESTINATION ${PYTHON_SITE}       # For Unix-like systems
        RUNTIME DESTINATION ${PYTHON_SITE}       # For Windows (safe to include)
    )
    message(STATUS "Python site-packages directory: ${PYTHON_SITE}")

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

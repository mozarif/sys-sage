set(CMAKE_INCLUDE_CURRENT_DIR ON)

#defines.hpp header with #cmakedefine 
configure_file(${PROJECT_SOURCE_DIR}/src/defines.hpp.in ${PROJECT_SOURCE_DIR}/src/defines.hpp)

include_directories(external_interfaces)
set(EXT_INTF "external_interfaces")
set(PY_BINDS "python-bindings")



set(SOURCES
    Component.cpp
    DataPath.cpp
    xml_dump.cpp
    xml_load.cpp
    ${EXT_INTF}/intel_pqos.cpp
    ${EXT_INTF}/proc_cpuinfo.cpp
    ${EXT_INTF}/nvidia_mig.cpp
    ${PY_BINDS}/sys-sage-bindings.cpp
    parsers/hwloc.cpp
    parsers/caps-numa-benchmark.cpp
    parsers/mt4g.cpp
    parsers/cccbench.cpp
    )

set(HEADERS
    sys-sage.hpp
    defines.hpp
    Component.hpp
    DataPath.hpp
    xml_dump.hpp
    xml_load.hpp
    parsers/hwloc.hpp
    parsers/caps-numa-benchmark.hpp
    parsers/mt4g.hpp
    parsers/cccbench.cpp
    )

# add_library(sys-sage SHARED ${SOURCES} ${HEADERS})

if(PYBIND)
    find_package(Python 3.13 COMPONENTS Interpreter Development REQUIRED)
    link_directories(${PYTHON_LIBRARY_DIRS})
    include_directories(${PYTHON_INCLUDE_DIRS})
    set(PYBIND11_FINDPYTHON ON)
    find_package(pybind11 CONFIG REQUIRED)
    execute_process(
    COMMAND "${PYTHON_EXECUTABLE}" -c "import sysconfig; print(sysconfig.get_paths()['purelib'])"
    OUTPUT_VARIABLE PYTHON_SITE
    OUTPUT_STRIP_TRAILING_WHITESPACE)
    add_library(syssage MODULE ${SOURCES} ${HEADERS}
    ${PY_BINDS}/sys-sage-bindings.cpp)
    target_link_libraries(syssage PUBLIC ${PYTHON_LIBRARIES} pybind11::module)
    set_target_properties(syssage PROPERTIES PREFIX "" OUTPUT_NAME "sys_sage")
    install(
        TARGETS syssage
        LIBRARY DESTINATION ${PYTHON_SITE} 
    )
    message(STATUS "Python site-packages directory: ${PYTHON_SITE}")

else()

    add_library(sys-sage SHARED ${SOURCES} ${HEADERS})
    target_include_directories(sys-sage PUBLIC  
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  
        $<INSTALL_INTERFACE:inc>
        $<INSTALL_INTERFACE:lib>
    )
    install(
        TARGETS sys-sage
        EXPORT sys-sage-targets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib/cmake/lib
        ARCHIVE DESTINATION lib/cmake/lib
    )
    #for spack
    install(
        EXPORT sys-sage-targets
        FILE sys-sage-targets.cmake
        DESTINATION lib/cmake/syssage
        NAMESPACE syssage::
    )
    install(DIRECTORY "."
        DESTINATION lib/cmake/inc
        FILES_MATCHING PATTERN "*.hpp")

    install(
        TARGETS sys-sage
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
    install(DIRECTORY "."
        DESTINATION inc
        FILES_MATCHING PATTERN "*.hpp")

endif()



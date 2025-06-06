set(CMAKE_INCLUDE_CURRENT_DIR ON)

#defines.hpp header with #cmakedefine 
configure_file(${PROJECT_SOURCE_DIR}/src/defines.hpp.in ${PROJECT_SOURCE_DIR}/src/defines.hpp)

include_directories(external_interfaces)
set(EXT_INTF "external_interfaces")

set(SOURCES
    Component.cpp
    DataPath.cpp
    xml_dump.cpp
    xml_load.cpp
    ${EXT_INTF}/intel_pqos.cpp
    ${EXT_INTF}/proc_cpuinfo.cpp
    ${EXT_INTF}/nvidia_mig.cpp
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

if(PY_SYS_SAGE)
    # find_package(Python 3.13 COMPONENTS Interpreter Development REQUIRED)
    link_directories(${PYTHON_LIBRARY_DIRS})
    include_directories(${PYTHON_INCLUDE_DIRS})
    set(PYBIND11_FINDPYTHON ON)
    # find_package(pybind11 CONFIG REQUIRED)
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

    set(PY_BINDS ${CMAKE_SOURCE_DIR}/src/python-bindings)
    message(STATUS "PY_BINDS=${PY_BINDS}")

    # This builds a Python extension module (NOT a regular .so/.dylib)
    pybind11_add_module(sys_sage MODULE 
        ${PY_BINDS}/sys-sage-bindings.cpp
        ${SOURCES}
    )

    target_link_libraries(sys_sage PUBLIC ${PYTHON_LIBRARIES} pybind11::module)
    # set_target_properties(sys-sage PROPERTIES PREFIX "" OUTPUT_NAME "py_sys_sage")
    set_target_properties(sys_sage PROPERTIES
        PREFIX ""
        OUTPUT_NAME "sys_sage"
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/python
    )
    install(
        TARGETS sys_sage
        LIBRARY DESTINATION ${PYTHON_SITE}       # For Unix-like systems
        RUNTIME DESTINATION ${PYTHON_SITE}       # For Windows (safe to include)
    )
    message(STATUS "Python site-packages directory: ${PYTHON_SITE}")

endif()
#else()

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

#endif()


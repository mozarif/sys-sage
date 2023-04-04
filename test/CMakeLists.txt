include_directories(../src) # The include path is not set in the sys-sage target because CMAKE_INCLUDE_CURRENT_DIR is used instead

add_subdirectory(ut)
add_executable(test test.cpp topology.cpp datapath.cpp hwloc.cpp gpu-topo.cpp caps-numa-benchmark.cpp cpuinfo.cpp export.cpp)
target_link_libraries(test PRIVATE ut sys-sage)
target_compile_definitions(test PRIVATE SYS_SAGE_TEST_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources")

if(${TEST_ASAN})
    target_compile_options(test PRIVATE -fsanitize=address -g3)
    target_link_options(test PRIVATE -fsanitize=address)
endif()

add_subdirectory(googletest)

add_executable(test test.cpp topology.cpp datapath.cpp export.cpp hwloc.cpp)
target_link_libraries(test PRIVATE sys-sage gtest gmock)
include_directories(../src) # The include path is not set in the sys-sage target because CMAKE_INCLUDE_CURRENT_DIR is used instead
target_compile_definitions(test PRIVATE SYS_SAGE_TEST_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources")

add_subdirectory(googletest)

add_executable(test test.cpp)
target_link_libraries(test PRIVATE sys-sage gtest gmock)
include_directories(../src) # The include path is not set in the sys-sage target because CMAKE_INCLUDE_CURRENT_DIR is used instead

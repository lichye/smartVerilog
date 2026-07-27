cmake_minimum_required(VERSION 3.16)

project(smartVerilog VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "" FORCE)
endif()

# Development tools live in build/bin/; the product itself lands at
# build/smart, so `cd build && ./smart` does what anyone would expect.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ctest needs this at the top level, or the subdirectory tests are invisible.
enable_testing()

add_subdirectory(third_party)
# Second argument: the subdirectory's build tree. Without it CMake would
# mirror the source layout and create build/smart AS A DIRECTORY, which is
# exactly where the binary wants to be.
add_subdirectory(smart smart-build)

cmake_minimum_required(VERSION 3.10)

project(mini-panel C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Output binaries to bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

add_subdirectory(src)
add_subdirectory(examples)

# Unit tests (requires -DBUILD_TESTING=ON)
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

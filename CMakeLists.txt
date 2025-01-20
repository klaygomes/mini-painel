cmake_minimum_required(VERSION 3.10)

# Set the project name
project(mini-panel)
# Set C version to C99
set(CMAKE_C_STANDARD 99)

# Set the output folder where your program will be created
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)


# Add the executable
add_subdirectory(src)


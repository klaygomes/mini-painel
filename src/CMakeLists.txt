add_subdirectory(components)

add_executable(panel
    main.c serial.c protocol.c image.c port_detect.c panel.c dashboard.c
)

target_include_directories(panel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(panel PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(panel PRIVATE xf_components)

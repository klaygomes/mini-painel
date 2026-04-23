add_subdirectory(components)

add_executable(panel
    main.c serial.c
    turing_protocol.c turing_image.c turing_port_detect.c turing_panel.c
    dashboard.c
)

target_include_directories(panel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(panel PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(panel PRIVATE xf_components)

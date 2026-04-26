add_subdirectory(theme)
add_subdirectory(draw)
add_subdirectory(components)

add_executable(panel
    main.c
    serial/serial.c
    protocol/turing_protocol.c
    protocol/turing_port_detect.c
    device/turing_panel.c
    device/panel_common.c
    image/image.c
    dashboard/dashboard.c
)

target_include_directories(panel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/serial
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol
    ${CMAKE_CURRENT_SOURCE_DIR}/device
    ${CMAKE_CURRENT_SOURCE_DIR}/image
    ${CMAKE_CURRENT_SOURCE_DIR}/dashboard
)
target_compile_options(panel PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(panel PRIVATE xf_components)

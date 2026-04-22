cmake_minimum_required(VERSION 3.10)

add_executable(test_panel
    test_panel.c
    vendor/unity.c
    fake_serial.c
    ${CMAKE_SOURCE_DIR}/src/panel.c
    ${CMAKE_SOURCE_DIR}/src/protocol.c
    ${CMAKE_SOURCE_DIR}/src/image.c
    ${CMAKE_SOURCE_DIR}/src/port_detect.c
)

target_include_directories(test_panel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_options(test_panel PRIVATE -Wall -Wextra)
add_test(NAME test_panel COMMAND test_panel)

add_executable(test_dashboard
    test_dashboard.c
    vendor/unity.c
    ${CMAKE_SOURCE_DIR}/src/dashboard.c
)

target_include_directories(test_dashboard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_options(test_dashboard PRIVATE -Wall -Wextra)
add_test(NAME test_dashboard COMMAND test_dashboard)

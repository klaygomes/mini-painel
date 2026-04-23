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

target_compile_options(test_panel PRIVATE -Wall -Wextra -Werror)
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
target_link_libraries(test_dashboard PRIVATE xf_components)
add_test(NAME test_dashboard COMMAND test_dashboard)

add_executable(test_components
    test_components.c
    vendor/unity.c
    ${CMAKE_SOURCE_DIR}/src/dashboard.c
)

target_include_directories(test_components PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_options(test_components PRIVATE -Wall -Wextra)
target_link_libraries(test_components PRIVATE xf_components)
add_test(NAME test_components COMMAND test_components)
set_tests_properties(test_components PROPERTIES
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(test_transition
    test_transition.c
    vendor/unity.c
    fake_serial.c
    ${CMAKE_SOURCE_DIR}/src/transition.c
    ${CMAKE_SOURCE_DIR}/src/panel.c
    ${CMAKE_SOURCE_DIR}/src/protocol.c
    ${CMAKE_SOURCE_DIR}/src/image.c
    ${CMAKE_SOURCE_DIR}/src/port_detect.c
)

target_include_directories(test_transition PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_options(test_transition PRIVATE -Wall -Wextra)
add_test(NAME test_transition COMMAND test_transition)

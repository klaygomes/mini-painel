cmake_minimum_required(VERSION 3.10)

set(SRC ${CMAKE_SOURCE_DIR}/src)
set(TEST_INCS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    ${SRC}
    ${SRC}/serial
    ${SRC}/protocol
    ${SRC}/device
    ${SRC}/image
    ${SRC}/dashboard
    ${SRC}/draw
    ${SRC}/theme
)

add_executable(test_panel
    test_panel.c
    vendor/unity.c
    fake_serial.c
    ${SRC}/device/panel.c
    ${SRC}/device/panel_common.c
    ${SRC}/protocol/protocol.c
    ${SRC}/image/image.c
    ${SRC}/protocol/port_detect.c
)

target_include_directories(test_panel PRIVATE ${TEST_INCS})
target_compile_options(test_panel PRIVATE -Wall -Wextra -Werror)
add_test(NAME test_panel COMMAND test_panel)

add_executable(test_dashboard
    test_dashboard.c
    vendor/unity.c
    ${SRC}/dashboard/dashboard.c
)

target_include_directories(test_dashboard PRIVATE ${TEST_INCS})
target_compile_options(test_dashboard PRIVATE -Wall -Wextra)
target_link_libraries(test_dashboard PRIVATE xf_components)
add_test(NAME test_dashboard COMMAND test_dashboard)

add_executable(test_components
    test_components.c
    vendor/unity.c
    ${SRC}/dashboard/dashboard.c
)

target_include_directories(test_components PRIVATE ${TEST_INCS})
target_compile_options(test_components PRIVATE -Wall -Wextra)
target_link_libraries(test_components PRIVATE xf_components)
add_test(NAME test_components COMMAND test_components)
set_tests_properties(test_components PROPERTIES
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(test_transition
    test_transition.c
    vendor/unity.c
    fake_serial.c
    ${SRC}/draw/transition.c
    ${SRC}/device/panel.c
    ${SRC}/device/panel_common.c
    ${SRC}/protocol/protocol.c
    ${SRC}/image/image.c
    ${SRC}/protocol/port_detect.c
)

target_include_directories(test_transition PRIVATE ${TEST_INCS})
target_compile_options(test_transition PRIVATE -Wall -Wextra)
add_test(NAME test_transition COMMAND test_transition)

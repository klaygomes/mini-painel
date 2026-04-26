set(SRC ${CMAKE_SOURCE_DIR}/src)

set(PANEL_SRCS
    ${SRC}/serial/serial.c
    ${SRC}/protocol/turing_protocol.c
    ${SRC}/image/image.c
    ${SRC}/protocol/turing_port_detect.c
    ${SRC}/device/turing_panel.c
    ${SRC}/device/panel_common.c
    ${SRC}/dashboard/dashboard.c
)

set(PANEL_INCS
    ${SRC}
    ${SRC}/serial
    ${SRC}/protocol
    ${SRC}/device
    ${SRC}/image
    ${SRC}/dashboard
    ${SRC}/draw
    ${SRC}/theme
)

add_executable(demo demo.c ${PANEL_SRCS})
target_include_directories(demo PRIVATE ${PANEL_INCS})
target_compile_options(demo PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(demo PRIVATE xf_components)

add_executable(live live.c ${PANEL_SRCS} ${SRC}/draw/transition.c)
target_include_directories(live PRIVATE ${PANEL_INCS})
target_compile_options(live PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(live PRIVATE xf_components m)

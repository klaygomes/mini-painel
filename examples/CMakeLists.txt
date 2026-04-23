set(SRC ${CMAKE_SOURCE_DIR}/src)

set(PANEL_SRCS
    ${SRC}/serial.c
    ${SRC}/turing_protocol.c
    ${SRC}/turing_image.c
    ${SRC}/turing_port_detect.c
    ${SRC}/turing_panel.c
    ${SRC}/dashboard.c
)

add_executable(demo demo.c ${PANEL_SRCS})
target_include_directories(demo PRIVATE ${SRC})
target_compile_options(demo PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(demo PRIVATE xf_components)

add_executable(live live.c ${PANEL_SRCS} ${SRC}/transition.c)
target_include_directories(live PRIVATE ${SRC})
target_compile_options(live PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(live PRIVATE xf_components m)

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
    ${SRC}/components
    ${SRC}/gfx
)

add_executable(demo demo.c ${PANEL_SRCS})
target_include_directories(demo PRIVATE ${PANEL_INCS})
target_compile_options(demo PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(demo PRIVATE xf_components)

add_executable(live live.c ${PANEL_SRCS} ${SRC}/draw/transition.c)
target_include_directories(live PRIVATE ${PANEL_INCS})
target_compile_options(live PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(live PRIVATE xf_components m)

add_executable(json_demo json_demo.c ${SRC}/dashboard/dashboard.c)
target_include_directories(json_demo PRIVATE
    ${PANEL_INCS}
    ${SRC}/json_api
    ${CMAKE_SOURCE_DIR}/vendor/mjson
)
target_compile_options(json_demo PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(json_demo PRIVATE xf_components xf_json_api)

set(MONGOOSE_SRC ${CMAKE_SOURCE_DIR}/vendor/mongoose/mongoose.c)

add_executable(panel_daemon
    panel_daemon.c
    ${PANEL_SRCS}
    ${SRC}/b64.c
    ${MONGOOSE_SRC}
)
target_include_directories(panel_daemon PRIVATE
    ${PANEL_INCS}
    ${SRC}/json_api
    ${CMAKE_SOURCE_DIR}/vendor/mjson
    ${CMAKE_SOURCE_DIR}/vendor/mongoose
)
target_compile_options(panel_daemon PRIVATE -Wall -Wextra -Wpedantic -Werror)
set_source_files_properties(${MONGOOSE_SRC} PROPERTIES COMPILE_FLAGS
    "-Wno-unused-parameter -Wno-sign-conversion -Wno-implicit-fallthrough \
     -Wno-cast-qual -Wno-unused-but-set-variable")
target_link_libraries(panel_daemon PRIVATE xf_components xf_json_api)

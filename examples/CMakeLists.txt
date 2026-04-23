set(SRC ${CMAKE_SOURCE_DIR}/src)

add_executable(demo
    demo.c
    ${SRC}/serial.c
    ${SRC}/protocol.c
    ${SRC}/image.c
    ${SRC}/port_detect.c
    ${SRC}/panel.c
    ${SRC}/dashboard.c
)

target_include_directories(demo PRIVATE ${SRC})
target_compile_options(demo PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(demo PRIVATE xf_components)

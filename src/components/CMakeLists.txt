file(GLOB COMPONENT_SOURCES "*.c")

add_library(xf_components STATIC ${COMPONENT_SOURCES})
target_compile_options(xf_components PRIVATE -Wall -Wextra -Werror)

target_include_directories(xf_components
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
           ${CMAKE_CURRENT_SOURCE_DIR}/../dashboard
)

# PUBLIC so that targets linking xf_components also get the draw and theme
# headers and libraries transitively.
target_link_libraries(xf_components PUBLIC xf_draw)

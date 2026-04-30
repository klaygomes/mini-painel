file(GLOB GFX_SOURCES CONFIGURE_DEPENDS "*.c")

add_library(xf_gfx STATIC ${GFX_SOURCES})
target_compile_options(xf_gfx PRIVATE -Wall -Wextra -Werror)
target_include_directories(xf_gfx PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
# PUBLIC so that targets linking xf_gfx also get draw headers transitively.
target_link_libraries(xf_gfx PUBLIC xf_draw)

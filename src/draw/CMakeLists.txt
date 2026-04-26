find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)

add_library(xf_draw STATIC draw.c)
target_compile_options(xf_draw PRIVATE -Wall -Wextra -Werror)
target_include_directories(xf_draw
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CAIRO_INCLUDE_DIRS}
)
# PUBLIC so that targets linking xf_draw also pull in Cairo at link time.
target_link_libraries(xf_draw PUBLIC xf_theme ${CAIRO_LINK_LIBRARIES})

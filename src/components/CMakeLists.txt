find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)

file(GLOB COMPONENT_SOURCES "*.c")

add_library(xf_components STATIC ${COMPONENT_SOURCES})
target_compile_options(xf_components PRIVATE -Wall -Wextra -Werror)

target_include_directories(xf_components
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CAIRO_INCLUDE_DIRS}
)

# PUBLIC so that targets linking xf_components also pull in Cairo at link time,
# which is required because static libraries do not embed their dependencies.
target_link_libraries(xf_components PUBLIC ${CAIRO_LINK_LIBRARIES})

file(GLOB JSON_KIND_SRC CONFIGURE_DEPENDS kinds/*.c)

add_library(xf_json_api STATIC
    json_api.c
    json_backend_dashboard.c
    json_registry.c
    json_kinds.c
    json_decode_util.c
    json_encode.c
    ${JSON_KIND_SRC}
    ${CMAKE_SOURCE_DIR}/vendor/mjson/mjson.c
)

target_include_directories(xf_json_api
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/vendor/mjson
            ${CMAKE_SOURCE_DIR}/src/dashboard
            ${CMAKE_SOURCE_DIR}/src/components
            ${CMAKE_SOURCE_DIR}/src/theme
            ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(xf_json_api PUBLIC xf_components)
target_compile_options(xf_json_api PRIVATE -Wall -Wextra -Werror -pedantic)
set_source_files_properties(${CMAKE_SOURCE_DIR}/vendor/mjson/mjson.c
    PROPERTIES COMPILE_FLAGS "-Wno-unused-but-set-variable"
)

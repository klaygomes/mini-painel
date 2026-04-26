set(XF_THEME "default" CACHE STRING
    "Active theme variant; maps to theme_<name>.c in this directory")

add_library(xf_theme STATIC
    theme.c
    theme_${XF_THEME}.c
)
target_compile_options(xf_theme PRIVATE -Wall -Wextra -Werror)
target_include_directories(xf_theme PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

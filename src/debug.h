#ifndef XF_DEBUG_H
#define XF_DEBUG_H

/*
 * DEBUG_LOG — compile-time-gated diagnostic tracing.
 *
 * Enabled by defining XF_ENABLE_DEBUG_LOG=1 at compile time:
 *
 *   cmake -DXF_ENABLE_DEBUG_LOG=ON ..
 *
 * When enabled, DEBUG_LOG(fmt, ...) writes a line to stderr with the
 * format:
 *
 *   [xf][<function>] <formatted message>\n
 *
 * When disabled (the default), the macro expands to an empty do-while
 * so the compiler sees no code and no side effects.  Arguments are not
 * evaluated, which means:
 *   - no runtime overhead
 *   - no unused-variable warnings under -Wunused
 *
 * Usage:
 *   DEBUG_LOG("surface status %d", cairo_surface_status(s));
 *   DEBUG_LOG("reached early-exit path");
 *
 * This macro is for internal diagnostics only.  Do not use it for
 * user-visible messages or in public headers.
 */

#include <stdio.h>

#ifdef XF_ENABLE_DEBUG_LOG
#define DEBUG_LOG(...)                          \
    do {                                        \
        fprintf(stderr, "[xf][%s] ", __func__); \
        fprintf(stderr, __VA_ARGS__);           \
        fputc('\n', stderr);                   \
    } while (0)
#else
#define DEBUG_LOG(...) do { } while (0)
#endif

#endif /* XF_DEBUG_H */

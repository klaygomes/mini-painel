#ifndef XF_STATUS_H
#define XF_STATUS_H

/*
 * xf_result_t — unified return type for the render pipeline.
 *
 * Every function in the draw, component, and dashboard layers that can
 * fail returns xf_result_t.  Callers must check the return value;
 * a non-zero result means the frame is in an undefined state and must
 * not be used.
 *
 * Convention:
 *   0   success  (XF_RES_OK)
 *   < 0 failure  (one of the XF_RES_ERR_* constants below)
 *
 * Error codes:
 *
 *   XF_RES_OK (0)
 *     Operation completed successfully.
 *
 *   XF_RES_ERR_INVALID_ARG (-1)
 *     A required pointer argument was NULL, or a numeric argument was
 *     outside its valid range.  Indicates a programming error at the
 *     call site; should never occur in production.
 *
 *   XF_RES_ERR_NOMEM (-2)
 *     A heap allocation (malloc/calloc) or a Cairo surface allocation
 *     failed.  The framebuffer or surface is in an unusable state.
 *     The caller should abort the current render cycle and retry later.
 *
 *   XF_RES_ERR_CAIRO (-3)
 *     A Cairo surface or context entered an error state (any status
 *     other than CAIRO_STATUS_SUCCESS or CAIRO_STATUS_NO_MEMORY).
 *     Typical causes: unsupported pixel format, corrupt surface data,
 *     or an invalid drawing operation.  Inspect with
 *     DEBUG_LOG / cairo_status_to_string() under XF_ENABLE_DEBUG_LOG.
 */
typedef enum {
    XF_RES_OK            =  0,
    XF_RES_ERR_INVALID_ARG = -1,
    XF_RES_ERR_NOMEM     = -2,
    XF_RES_ERR_CAIRO     = -3
} xf_result_t;

#endif /* XF_STATUS_H */

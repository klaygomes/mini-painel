/**
 * @file comp_base.h
 * @brief Shared component type definitions and helper wrappers.
 */

#pragma once

#include <stdint.h>
#include "draw.h"
#include "layout.h"

/**
 * @brief A dashboard component: optional fetch plus required render behavior.
 *
 * The dashboard stores pointers to components but does not own their context.
 */
typedef struct xf_component xf_component_t;
struct xf_component {
    /** Called once per frame before render() to refresh self->ctx. */
    int  (*fetch)(xf_component_t *self);

    /** Draws into the supplied RGB888 buffer. */
    void (*render)(xf_component_t *self, uint8_t *buf, int width, int height);

    /** Caller-owned context passed into fetch() and render(). */
    void *ctx;

    /** Set non-zero by the caller when the component output changes. */
    int dirty;
};

/**
 * @brief Create a pure rendering component with no fetch step.
 *
 * @param render_fn Component render callback.
 */
#define XF_COMPONENT(render_fn) \
    { NULL, (render_fn), NULL, 0 }

/**
 * @brief Create a component that renders from a caller-owned context pointer.
 *
 * @param render_fn Component render callback.
 * @param ctx       Caller-owned context pointer.
 */
#define XF_COMPONENT_DATA(render_fn, ctx) \
    { NULL, (render_fn), (ctx), 0 }

/**
 * @brief Create a live component with a per-frame fetch step.
 *
 * @param fetch_fn  Component fetch callback.
 * @param render_fn Component render callback.
 * @param ctx       Caller-owned context pointer.
 */
#define XF_COMPONENT_LIVE(fetch_fn, render_fn, ctx) \
    { (fetch_fn), (render_fn), (ctx), 0 }

/**
 * @brief Invoke a component fetch callback if present.
 *
 * @param comp Component instance.
 */
void comp_fetch(xf_component_t *comp);

/**
 * @brief Invoke a component render callback.
 *
 * @param comp Component instance.
 * @param buf  RGB888 output buffer.
 * @param w    Component width.
 * @param h    Component height.
 */
void comp_render(xf_component_t *comp, uint8_t *buf, int w, int h);

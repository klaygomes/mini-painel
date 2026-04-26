/**
 * @file comp_sparkline.h
 * @brief Filled area sparkline chart with a title and current value label.
 */

#pragma once
#include "comp_base.h"

#define COMP_SPARKLINE_HEIGHT     50
#define COMP_SPARKLINE_MAX_POINTS 60

/**
 * @brief Data for the sparkline chart component.
 *
 * Points should be normalised to [0.0, 1.0] so the render function can scale
 * them to the available height without knowing the domain.
 */
typedef struct {
    char  title[32];                          /**< Chart label, e.g. "req/s"   */
    char  value[16];                          /**< Current reading, e.g. "412" */
    float points[COMP_SPARKLINE_MAX_POINTS];  /**< Data series, normalised 0-1 */
    int   count;                              /**< Number of valid points       */
} comp_sparkline_data_t;

/**
 * @brief Create a sparkline chart component bound to @p data.
 *
 * @param data  Populated data struct; must not be NULL.
 */
xf_component_t comp_sparkline_create(comp_sparkline_data_t *data);

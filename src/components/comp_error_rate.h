/**
 * @file comp_error_rate.h
 * @brief Bar histogram of recent error rate, colour-coded by severity threshold.
 *
 * Bars at index >= 10 use the danger colour, >= 7 use warning, otherwise info.
 * The caller normalises bar heights to [0.0, 1.0].
 */

#pragma once
#include "../dashboard.h"
#include "layout.h"

#define COMP_ERROR_RATE_HEIGHT   62
#define COMP_ERROR_RATE_MAX_BARS 16

/**
 * @brief Data for the error rate histogram component.
 */
typedef struct {
    char  title[32];                         /**< Label, e.g. "Error rate"      */
    char  value[16];                         /**< Current rate, e.g. "0.3%"     */
    float bars[COMP_ERROR_RATE_MAX_BARS];    /**< Normalised bar heights [0, 1] */
    int   count;                             /**< Number of bars to render      */
} comp_error_rate_data_t;

/**
 * @brief Create an error rate histogram component bound to @p data.
 *
 * @param data  Populated data struct; must not be NULL.
 */
xf_component_t comp_error_rate_create(comp_error_rate_data_t *data);

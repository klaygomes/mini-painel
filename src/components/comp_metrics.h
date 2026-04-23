/**
 * @file comp_metrics.h
 * @brief A row of small metric cards, each showing a label and a value.
 */

#pragma once
#include "../dashboard.h"
#include "layout.h"

#define COMP_METRICS_HEIGHT    46
#define COMP_METRICS_MAX_CARDS  4

/**
 * @brief One metric card (label + value pair).
 */
typedef struct {
    char label[32]; /**< Short all-caps label, e.g. "UPTIME"  */
    char value[32]; /**< Displayed value,        e.g. "99.94%" */
} comp_metric_card_t;

/**
 * @brief Data for the metrics component.
 */
typedef struct {
    comp_metric_card_t cards[COMP_METRICS_MAX_CARDS]; /**< Card definitions  */
    int                count;                          /**< Number of cards   */
} comp_metrics_data_t;

/**
 * @brief Create a metrics component bound to @p data.
 *
 * @param data  Populated data struct; must not be NULL.
 */
xf_component_t comp_metrics_create(comp_metrics_data_t *data);

/**
 * @file comp_sla_gauge.h
 * @brief Horizontal progress bars showing SLA attainment per service.
 */

#pragma once
#include "../dashboard.h"
#include "draw.h"
#include "layout.h"

#define COMP_SLA_GAUGE_HEIGHT   (LAY_HEADER_H + 3 * LAY_ROW_MD)
#define COMP_SLA_GAUGE_MAX_ROWS  3

/**
 * @brief One SLA gauge row.
 */
typedef struct {
    char      label[32]; /**< Service label, e.g. "API Gateway"  */
    char      value[16]; /**< Displayed value, e.g. "99.94%"     */
    float     percent;   /**< Fill amount in [0.0, 100.0]        */
    xf_rgba_t bar;       /**< Progress bar fill colour           */
} comp_sla_row_t;

/**
 * @brief Data for the SLA gauge component.
 */
typedef struct {
    char           title[32];                       /**< Section heading    */
    comp_sla_row_t rows[COMP_SLA_GAUGE_MAX_ROWS];   /**< Gauge definitions  */
    int            count;                           /**< Number of gauges   */
} comp_sla_gauge_data_t;

/**
 * @brief Create an SLA gauge component bound to @p data.
 *
 * @param data  Populated data struct; must not be NULL.
 */
xf_component_t comp_sla_gauge_create(comp_sla_gauge_data_t *data);

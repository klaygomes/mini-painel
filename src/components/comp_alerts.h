/**
 * @file comp_alerts.h
 * @brief List of recent system alerts with per-row severity colours.
 */

#pragma once
#include "../dashboard.h"
#include "draw.h"
#include "layout.h"

#define COMP_ALERTS_HEIGHT   (LAY_HEADER_H + 3 * LAY_ROW_ALERT + 2 * LAY_GAP_SM)
#define COMP_ALERTS_MAX_ROWS  3

/**
 * @brief One alert entry.
 *
 * The caller assigns severity-specific theme colours at creation time,
 * keeping the render function free of colour-selection logic.
 */
typedef struct {
    char      message[64]; /**< Alert description (may be clipped)  */
    char      time[16];    /**< Age string, e.g. "3m ago"           */
    xf_rgba_t dot;         /**< Severity indicator dot colour       */
    xf_rgba_t row_bg;      /**< Row background tint                 */
} comp_alert_row_t;

/**
 * @brief Data for the alerts list component.
 */
typedef struct {
    char             title[32];                   /**< Section heading       */
    comp_alert_row_t rows[COMP_ALERTS_MAX_ROWS];  /**< Alert entries         */
    int              count;                       /**< Number of active rows */
} comp_alerts_data_t;

/**
 * @brief Create an alerts list component bound to @p data.
 *
 * @param data  Populated data struct; must not be NULL.
 */
xf_component_t comp_alerts_create(comp_alerts_data_t *data);

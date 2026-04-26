/**
 * @file comp_outages.h
 * @brief Scrollable list of active service outages with severity indicators.
 */

#pragma once
#include "comp_base.h"

#define COMP_OUTAGES_HEIGHT   (LAY_HEADER_H + 3 * LAY_ROW_LG + 2 * LAY_GAP_MD)
#define COMP_OUTAGES_MAX_ROWS  3

/**
 * @brief One row in the outage list.
 *
 * Color fields let the caller assign severity-specific theme values at
 * creation time so the render function stays free of conditional logic.
 */
typedef struct {
    char      service[32];  /**< Service name, e.g. "auth-service"   */
    char      duration[16]; /**< Time affected, e.g. "2h 14m"        */
    char      status[32];   /**< Outcome label, e.g. "partial outage" */
    xf_rgba_t row_bg;       /**< Row background tint                  */
    xf_rgba_t pill_bg;      /**< Status pill fill colour              */
    xf_rgba_t pill_fg;      /**< Status pill text colour              */
    xf_rgba_t title_fg;     /**< Service name text colour             */
    xf_rgba_t dot;          /**< Severity indicator dot colour        */
} comp_outage_row_t;

/**
 * @brief Data for the outages list component.
 */
typedef struct {
    char              title[32];                     /**< Section heading       */
    comp_outage_row_t rows[COMP_OUTAGES_MAX_ROWS];   /**< Row definitions       */
    int               count;                         /**< Number of active rows */
} comp_outages_data_t;

/**
 * @brief Create an outages list component bound to @p data.
 *
 * @param data  Populated data struct; must not be NULL.
 */
xf_component_t comp_outages_create(comp_outages_data_t *data);

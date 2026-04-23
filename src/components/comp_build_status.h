/**
 * @file comp_build_status.h
 * @brief Single-row CI build pipeline status with branch, ID, and outcome pill.
 */

#pragma once
#include "../dashboard.h"
#include "draw.h"
#include "layout.h"

#define COMP_BUILD_STATUS_HEIGHT 40

/**
 * @brief Data for the build status component.
 */
typedef struct {
    char      branch[32];   /**< Branch name, e.g. "main"           */
    char      build_id[16]; /**< Build identifier, e.g. "#1234"     */
    char      duration[16]; /**< Build duration, e.g. "2m 34s"      */
    char      status[16];   /**< Outcome label, e.g. "passing"      */
    xf_rgba_t status_color; /**< Pill background colour             */
    xf_rgba_t status_fg;    /**< Pill text colour                   */
} comp_build_status_data_t;

/**
 * @brief Create a build status component bound to @p data.
 *
 * @param data  Populated data struct; must not be NULL.
 */
xf_component_t comp_build_status_create(comp_build_status_data_t *data);

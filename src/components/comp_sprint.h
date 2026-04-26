/**
 * @file comp_sprint.h
 * @brief Sprint progress bar with story-point totals and days remaining.
 */

#pragma once
#include "comp_base.h"

#define COMP_SPRINT_HEIGHT 48

/**
 * @brief Data for the sprint progress component.
 */
typedef struct {
    char  title[32];          /**< Sprint name, e.g. "Sprint 24"      */
    char  progress_label[32]; /**< Points label, e.g. "8 / 13 pts"    */
    char  time_left[16];      /**< Remaining time, e.g. "3d left"     */
    float percent;            /**< Completion fraction in [0.0, 1.0]  */
} comp_sprint_data_t;

/**
 * @brief Create a sprint progress component bound to @p data.
 *
 * @param data  Populated data struct; must not be NULL.
 */
xf_component_t comp_sprint_create(comp_sprint_data_t *data);

/**
 * @file comp_schedule.h
 * @brief Timeline list of upcoming scheduled events.
 */

#pragma once
#include "../dashboard.h"
#include "draw.h"

#define COMP_SCHEDULE_HEIGHT   84
#define COMP_SCHEDULE_MAX_ROWS  4

/**
 * @brief One row in the schedule.
 */
typedef struct {
    char      time[16];  /**< Time window, e.g. "09:00-10:00"        */
    char      event[64]; /**< Event description, e.g. "Deploy freeze" */
    xf_rgba_t bar;       /**< Left accent bar colour                  */
} comp_schedule_row_t;

/**
 * @brief Data for the schedule list component.
 */
typedef struct {
    char                title[32];                       /**< Section heading       */
    comp_schedule_row_t rows[COMP_SCHEDULE_MAX_ROWS];    /**< Row definitions       */
    int                 count;                           /**< Number of active rows */
} comp_schedule_data_t;

/**
 * @brief Create a schedule list component bound to @p data.
 *
 * @param data  Populated data struct; must not be NULL.
 */
xf_component_t comp_schedule_create(comp_schedule_data_t *data);

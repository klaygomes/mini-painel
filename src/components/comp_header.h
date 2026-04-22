/**
 * @file comp_header.h
 * @brief Top bar showing the current date and a live-status indicator.
 */

#pragma once
#include "../dashboard.h"
#include "draw.h"

#define COMP_HEADER_HEIGHT 16

/**
 * @brief Data for the header bar component.
 */
typedef struct {
    char      date[32];       /**< Date string displayed on the left, e.g. "Mon · Apr 20" */
    char      status_text[32];/**< Status label on the right, e.g. "2 active"             */
    xf_rgba_t status_dot;     /**< Colour of the indicator dot beside the status text      */
} comp_header_data_t;

/**
 * @brief Create a header bar component bound to @p data.
 *
 * The dashboard holds the pointer; the caller owns the lifetime of @p data.
 *
 * @param data  Populated data struct; must not be NULL.
 */
xf_component_t comp_header_create(comp_header_data_t *data);

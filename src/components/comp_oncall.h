/**
 * @file comp_oncall.h
 * @brief Card showing the current on-call engineer with avatar and contact.
 */

#pragma once
#include "../dashboard.h"
#include "draw.h"

#define COMP_ONCALL_HEIGHT 32

/**
 * @brief Data for the on-call card component.
 */
typedef struct {
    char      initials[4];    /**< Two-letter avatar initials, e.g. "MR"         */
    char      name[64];       /**< Full name, e.g. "Maya Rodriguez"              */
    char      role[64];       /**< Role / team string, e.g. "Primary · platform" */
    char      phone[16];      /**< Extension or number, e.g. "x4172"             */
    xf_rgba_t avatar_color;   /**< Avatar circle fill colour                      */
} comp_oncall_data_t;

/**
 * @brief Create an on-call card component bound to @p data.
 *
 * @param data  Populated data struct; must not be NULL.
 */
xf_component_t comp_oncall_create(comp_oncall_data_t *data);

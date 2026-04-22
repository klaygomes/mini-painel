/**
 * @file comp_spacer.h
 * @brief Empty vertical gap used to add breathing room between rows.
 */

#pragma once
#include "../dashboard.h"

#define COMP_SPACER_HEIGHT 8

/**
 * @brief Create a blank spacer component.
 */
xf_component_t comp_spacer_create(void);

/**
 * @file comp_deploy.h
 * @brief Single-line deploy status showing branch, age, and outcome label.
 */

#pragma once
#include "../dashboard.h"
#include "layout.h"

#define COMP_DEPLOY_HEIGHT 28

/**
 * @brief Data for the deploy status component.
 */
typedef struct {
    char branch[32];   /**< Branch + commit ref, e.g. "main@a3f2c1"    */
    char time_ago[16]; /**< Time since deploy, e.g. "8m"               */
    char label[64];    /**< Human-readable outcome, e.g. "prod green"  */
} comp_deploy_data_t;

/**
 * @brief Create a deploy status component bound to @p data.
 *
 * @param data  Populated data struct; must not be NULL.
 */
xf_component_t comp_deploy_create(comp_deploy_data_t *data);

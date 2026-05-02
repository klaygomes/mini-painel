/**
 * @file comp_deploy.h
 * @brief Single-line deploy status showing branch, age, and outcome label.
 */

#pragma once
#include "comp_base.h"

#define COMP_DEPLOY_HEIGHT 28

/**
 * @brief Build/deploy status indicator colour.
 */
typedef enum {
    COMP_DEPLOY_STATUS_BUILDING = 0, /**< In progress — yellow. */
    COMP_DEPLOY_STATUS_SUCCESS,      /**< Passed — green.       */
    COMP_DEPLOY_STATUS_FAILED,       /**< Failed — red.         */
} comp_deploy_status_t;

/**
 * @brief Data for the deploy status component.
 */
typedef struct {
    char branch[32];             /**< Branch + commit ref, e.g. "main@a3f2c1"    */
    char time_ago[16];           /**< Time since deploy, e.g. "8m"               */
    char label[64];              /**< Human-readable outcome, e.g. "prod green"  */
    comp_deploy_status_t status; /**< Dot colour: building/success/failed.        */
} comp_deploy_data_t;

/**
 * @brief Create a deploy status component bound to @p data.
 *
 * @param data  Populated data struct; must not be NULL.
 */
xf_component_t comp_deploy_create(comp_deploy_data_t *data);

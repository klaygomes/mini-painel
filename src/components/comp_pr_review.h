/**
 * @file comp_pr_review.h
 * @brief List of pull requests pending review, with age and approval count.
 */

#pragma once
#include "comp_base.h"

#define COMP_PR_REVIEW_HEIGHT   (LAY_HEADER_H + 3 * LAY_ROW_MD + 2 * LAY_GAP_SM)
#define COMP_PR_REVIEW_MAX_ROWS  3

/**
 * @brief One row in the PR review list.
 */
typedef struct {
    char      initials[4];  /**< Author avatar initials, e.g. "AR"   */
    char      title[64];    /**< PR title (clipped to fit the row)    */
    char      age[16];      /**< Time open, e.g. "2d"                 */
    int       reviews;      /**< Number of approvals received so far  */
    xf_rgba_t avatar_color; /**< Avatar circle fill colour            */
} comp_pr_row_t;

/**
 * @brief Data for the PR review list component.
 */
typedef struct {
    char          title[32];                      /**< Section heading       */
    comp_pr_row_t rows[COMP_PR_REVIEW_MAX_ROWS];  /**< Row definitions       */
    int           count;                          /**< Number of active rows */
} comp_pr_review_data_t;

/**
 * @brief Create a PR review list component bound to @p data.
 *
 * @param data  Populated data struct; must not be NULL.
 */
xf_component_t comp_pr_review_create(comp_pr_review_data_t *data);

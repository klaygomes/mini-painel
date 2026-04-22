/**
 * @file comp_team_status.h
 * @brief Row of avatar circles showing team member online / offline presence.
 */

#pragma once
#include "../dashboard.h"
#include "draw.h"

#define COMP_TEAM_STATUS_HEIGHT      44
#define COMP_TEAM_STATUS_MAX_MEMBERS  6

/**
 * @brief One team member entry.
 */
typedef struct {
    char      initials[4];  /**< Two-letter avatar initials, e.g. "AR" */
    char      name[32];     /**< Full name for the text list below avatars */
    xf_rgba_t avatar_color; /**< Avatar circle fill colour               */
    int       online;       /**< 1 = online (green dot), 0 = offline     */
} comp_team_member_t;

/**
 * @brief Data for the team status component.
 */
typedef struct {
    char               title[32];                             /**< Section heading          */
    comp_team_member_t members[COMP_TEAM_STATUS_MAX_MEMBERS]; /**< Member definitions       */
    int                count;                                 /**< Number of active members */
} comp_team_status_data_t;

/**
 * @brief Create a team status component bound to @p data.
 *
 * @param data  Populated data struct; must not be NULL.
 */
xf_component_t comp_team_status_create(comp_team_status_data_t *data);

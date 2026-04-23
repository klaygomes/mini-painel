/**
 * @file comp_checklist.h
 * @brief Checklist with done / undone items and strikethrough on completed text.
 */

#pragma once
#include "../dashboard.h"
#include "layout.h"

#define COMP_CHECKLIST_HEIGHT    (LAY_HEADER_H + 4 * LAY_ROW_SM)
#define COMP_CHECKLIST_MAX_ITEMS  4

/**
 * @brief One checklist item.
 */
typedef struct {
    char item[64]; /**< Item text                                  */
    int  done;     /**< 1 = checked with strikethrough; 0 = pending */
} comp_checklist_item_t;

/**
 * @brief Data for the checklist component.
 */
typedef struct {
    char                  title[32];                        /**< Section heading   */
    comp_checklist_item_t items[COMP_CHECKLIST_MAX_ITEMS];  /**< Item definitions  */
    int                   count;                            /**< Number of items   */
} comp_checklist_data_t;

/**
 * @brief Create a checklist component bound to @p data.
 *
 * @param data  Populated data struct; must not be NULL.
 */
xf_component_t comp_checklist_create(comp_checklist_data_t *data);

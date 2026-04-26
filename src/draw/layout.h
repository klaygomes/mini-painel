#ifndef LAYOUT_H
#define LAYOUT_H

/* Vertical row heights. */
#define LAY_HEADER_H  18   /* section label row */
#define LAY_TITLE_H   16   /* inline title bar (sparkline, error rate) */
#define LAY_ROW_SM    19   /* checklist, schedule */
#define LAY_ROW_ALERT 20   /* alert rows */
#define LAY_ROW_MD    22   /* SLA gauge, PR review */
#define LAY_ROW_LG    26   /* outage rows */

/* Gaps applied between repeated rows, not after the last one. */
#define LAY_GAP_SM     2
#define LAY_GAP_MD     4

/* Universal horizontal padding. */
#define LAY_PAD_X      8

#endif /* LAYOUT_H */

#ifndef XF_JSON_KIND_H
#define XF_JSON_KIND_H

#include <stddef.h>
#include "comp_base.h"

typedef int            (*xf_decode_fn)(const char *json, int len, void *data, int patch);
typedef int            (*xf_encode_fn)(const void *data, char *buf, size_t cap);
typedef xf_component_t (*xf_create_fn)(void *data);

typedef struct {
    const char  *name;       /* "header", "alerts", ...                   */
    size_t       data_size;  /* 0 for stateless                           */
    int          height;
    xf_create_fn create;
    xf_decode_fn decode;     /* may be a no-op for stateless              */
    xf_encode_fn encode;     /* serialises data struct back to JSON obj   */
    int          stateless;  /* 1 -> dispatcher allocates no data block   */
} xf_kind_def_t;

const xf_kind_def_t *xf_kind_lookup(const char *name, int name_len);
int                  xf_kind_count(void);
const xf_kind_def_t *xf_kind_at(int i);

#endif

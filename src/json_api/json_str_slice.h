#ifndef XF_JSON_STR_SLICE_H
#define XF_JSON_STR_SLICE_H

/*
 * A non-owning view of a JSON/string buffer: pointer plus byte length.
 * Used throughout the decode layer to avoid repeating (const char *, int)
 * parameter pairs in every function signature.
 */
typedef struct {
    const char *ptr;
    int         len;
} xf_str_slice_t;

#endif /* XF_JSON_STR_SLICE_H */

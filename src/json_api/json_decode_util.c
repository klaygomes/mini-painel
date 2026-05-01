#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "json_decode_util.h"
#include "mjson.h"
#include "debug.h"

static int utf8_is_valid(const unsigned char *s, size_t len)
{
    size_t i = 0;

    while (i < len) {
        unsigned char c = s[i++];

        if (c <= 0x7F)
            continue;

        if (c >= 0xC2 && c <= 0xDF) {
            if (i >= len || (s[i] & 0xC0) != 0x80)
                return 0;
            i += 1;
            continue;
        }

        if (c == 0xE0) {
            if (i + 1 >= len)
                return 0;
            if (s[i] < 0xA0 || s[i] > 0xBF || (s[i + 1] & 0xC0) != 0x80)
                return 0;
            i += 2;
            continue;
        }

        if (c >= 0xE1 && c <= 0xEC) {
            if (i + 1 >= len)
                return 0;
            if ((s[i] & 0xC0) != 0x80 || (s[i + 1] & 0xC0) != 0x80)
                return 0;
            i += 2;
            continue;
        }

        if (c == 0xED) {
            if (i + 1 >= len)
                return 0;
            if (s[i] < 0x80 || s[i] > 0x9F || (s[i + 1] & 0xC0) != 0x80)
                return 0;
            i += 2;
            continue;
        }

        if (c >= 0xEE && c <= 0xEF) {
            if (i + 1 >= len)
                return 0;
            if ((s[i] & 0xC0) != 0x80 || (s[i + 1] & 0xC0) != 0x80)
                return 0;
            i += 2;
            continue;
        }

        if (c == 0xF0) {
            if (i + 2 >= len)
                return 0;
            if (s[i] < 0x90 || s[i] > 0xBF ||
                (s[i + 1] & 0xC0) != 0x80 || (s[i + 2] & 0xC0) != 0x80)
                return 0;
            i += 3;
            continue;
        }

        if (c >= 0xF1 && c <= 0xF3) {
            if (i + 2 >= len)
                return 0;
            if ((s[i] & 0xC0) != 0x80 || (s[i + 1] & 0xC0) != 0x80 ||
                (s[i + 2] & 0xC0) != 0x80)
                return 0;
            i += 3;
            continue;
        }

        if (c == 0xF4) {
            if (i + 2 >= len)
                return 0;
            if (s[i] < 0x80 || s[i] > 0x8F ||
                (s[i + 1] & 0xC0) != 0x80 || (s[i + 2] & 0xC0) != 0x80)
                return 0;
            i += 3;
            continue;
        }

        return 0;
    }

    return 1;
}

static size_t latin1_to_utf8(char *dst, size_t dst_size,
                             const unsigned char *src, size_t src_len)
{
    size_t out = 0;

    if (dst_size == 0)
        return 0;

    for (size_t i = 0; i < src_len; i++) {
        unsigned char b = src[i];
        if (b < 0x80) {
            if (out + 1 >= dst_size)
                break;
            dst[out++] = (char)b;
        } else {
            if (out + 2 >= dst_size)
                break;
            dst[out++] = (char)(0xC0u | (b >> 6));
            dst[out++] = (char)(0x80u | (b & 0x3Fu));
        }
    }

    dst[out] = '\0';
    return out;
}

int xf_decode_str(const char *s, int len, const char *path,
                  char *dst, size_t dst_size, int patch)
{
    char tmp[512];
    int  n;

    (void)patch;

    if (dst_size == 0)
        return -1;

    n = mjson_get_string(s, len, path, tmp, (int)sizeof(tmp));
    if (n < 0) {
        /* Path not found returns -1 from mjson_get_string too,
         * but so does a type mismatch. We treat absence as success. */
        const char *tok;
        int         toklen;
        int         tok_type = mjson_find(s, len, path, &tok, &toklen);
        if (tok_type == MJSON_TOK_INVALID)
            return 0; /* absent */
        return -1;   /* present but wrong type */
    }

    if (utf8_is_valid((const unsigned char *)tmp, (size_t)n)) {
        /* Truncate gracefully. */
        if ((size_t)n >= dst_size)
            n = (int)(dst_size - 1);
        memcpy(dst, tmp, (size_t)n);
        dst[n] = '\0';
    } else {
        latin1_to_utf8(dst, dst_size, (const unsigned char *)tmp, (size_t)n);
        DEBUG_LOG("normalized non-UTF8 string at path '%s'", path);
    }

    return 0;
}

static int hex_nibble(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

static int parse_hex_byte(const char *s)
{
    int hi = hex_nibble(s[0]);
    int lo = hex_nibble(s[1]);
    if (hi < 0 || lo < 0)
        return -1;
    return (hi << 4) | lo;
}

int xf_decode_color(const char *s, int len, const char *path,
                    xf_rgba_t *out, int patch)
{
    char buf[16];
    int  n;
    int  r, g, b;
    double a = 1.0;

    (void)patch;

    n = mjson_get_string(s, len, path, buf, (int)sizeof(buf));
    if (n < 0) {
        const char *tok;
        int         toklen;
        int         tok_type = mjson_find(s, len, path, &tok, &toklen);
        if (tok_type == MJSON_TOK_INVALID)
            return 0;
        return -1;
    }

    if (buf[0] != '#')
        return -1;

    if (n == 4) {
        /* #RGB -> #RRGGBB expansion */
        int rn = hex_nibble(buf[1]);
        int gn = hex_nibble(buf[2]);
        int bn = hex_nibble(buf[3]);
        if (rn < 0 || gn < 0 || bn < 0)
            return -1;
        r = (rn << 4) | rn;
        g = (gn << 4) | gn;
        b = (bn << 4) | bn;
    } else if (n == 7) {
        /* #RRGGBB */
        r = parse_hex_byte(buf + 1);
        g = parse_hex_byte(buf + 3);
        b = parse_hex_byte(buf + 5);
        if (r < 0 || g < 0 || b < 0)
            return -1;
    } else if (n == 9) {
        /* #RRGGBBAA */
        int aa;
        r  = parse_hex_byte(buf + 1);
        g  = parse_hex_byte(buf + 3);
        b  = parse_hex_byte(buf + 5);
        aa = parse_hex_byte(buf + 7);
        if (r < 0 || g < 0 || b < 0 || aa < 0)
            return -1;
        a = aa / 255.0;
    } else {
        return -1;
    }

    out->r = r / 255.0;
    out->g = g / 255.0;
    out->b = b / 255.0;
    out->a = a;
    return 0;
}

int xf_decode_float(const char *s, int len, const char *path,
                    float *out, int patch)
{
    double v;
    (void)patch;
    if (!mjson_get_number(s, len, path, &v)) {
        const char *tok;
        int         toklen;
        if (mjson_find(s, len, path, &tok, &toklen) == MJSON_TOK_INVALID)
            return 0;
        return -1;
    }
    *out = (float)v;
    return 0;
}

int xf_decode_int(const char *s, int len, const char *path,
                  int *out, int patch)
{
    double v;
    (void)patch;
    if (!mjson_get_number(s, len, path, &v)) {
        const char *tok;
        int         toklen;
        if (mjson_find(s, len, path, &tok, &toklen) == MJSON_TOK_INVALID)
            return 0;
        return -1;
    }
    *out = (int)v;
    return 0;
}

int xf_decode_bool(const char *s, int len, const char *path,
                   int *out, int patch)
{
    (void)patch;
    if (!mjson_get_bool(s, len, path, out)) {
        const char *tok;
        int         toklen;
        if (mjson_find(s, len, path, &tok, &toklen) == MJSON_TOK_INVALID)
            return 0;
        return -1;
    }
    return 0;
}

int xf_decode_float_array(const char *s, int len, const char *path,
                          float *dst, int max_count, int *out_count,
                          int patch)
{
    const char *arr;
    int         arr_len;
    int         tok_type;
    int         off, koff, klen, voff, vlen, vtype;
    int         count = 0;

    (void)patch;

    tok_type = mjson_find(s, len, path, &arr, &arr_len);
    if (tok_type == MJSON_TOK_INVALID)
        return 0;
    if (tok_type != MJSON_TOK_ARRAY)
        return -1;

    off = 0;
    while ((off = mjson_next(arr, arr_len, off,
                             &koff, &klen, &voff, &vlen, &vtype)) != 0) {
        double v;
        if (count >= max_count)
            break;
        if (vtype != MJSON_TOK_NUMBER)
            return -1;
        if (!mjson_get_number(arr + voff, vlen, "$", &v))
            return -1;
        dst[count++] = (float)v;
    }
    if (out_count)
        *out_count = count;
    return 0;
}

int xf_decode_object_array(const char *s, int len, const char *path,
                           void *array_base, size_t stride, int max_count,
                           int *out_count, xf_elem_decode_fn fn, int patch)
{
    const char *arr;
    int         arr_len;
    int         tok_type;
    int         off, koff, klen, voff, vlen, vtype;
    int         count = 0;

    (void)patch;

    tok_type = mjson_find(s, len, path, &arr, &arr_len);
    if (tok_type == MJSON_TOK_INVALID)
        return 0;
    if (tok_type != MJSON_TOK_ARRAY)
        return -1;

    off = 0;
    while ((off = mjson_next(arr, arr_len, off,
                             &koff, &klen, &voff, &vlen, &vtype)) != 0) {
        void *elem;
        if (count >= max_count)
            break;
        if (vtype != MJSON_TOK_OBJECT)
            return -1;
        elem = (char *)array_base + (size_t)count * stride;
        if (fn(arr + voff, vlen, elem, patch) < 0)
            return -1;
        count++;
    }
    if (out_count)
        *out_count = count;
    return 0;
}

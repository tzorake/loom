#ifndef TZFIXED_H
#define TZFIXED_H

//
// tzfixed.h - Arbitrary precision fixed-point numbers (requires tzbigint.h)
//
// The sign representation used by the underlying bigint is selected at
// compile time via TZBIGINT_IMPL (see tzbigint.h).
//
// Usage:
//   #define TZBIGINT_IMPLEMENTATION
//   #include "tzbigint.h"
//   #define TZFIXED_IMPLEMENTATION
//   #include "tzfixed.h"
//
// Options:
//   #define fixed_assert(expr)  // Custom assert (default: assert(expr))
//

#include <stdint.h>
#include <stddef.h>

#ifndef TZBIGINT_H
#  error "tzfixed.h requires tzbigint.h to be included first"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fixed_t fixed_t;

fixed_t* fixed_create(int frac_bits);
fixed_t* fixed_from_double(double value, int frac_bits);
fixed_t* fixed_from_string(const char* str, int frac_bits);
fixed_t* fixed_clone(const fixed_t* f);
void     fixed_destroy(fixed_t* f);

int      fixed_get_frac_bits(const fixed_t* f);
bigint_t* fixed_get_numerator(const fixed_t* f); // borrowed reference, do not free

double fixed_to_double(const fixed_t* f);
char*  fixed_to_string(const fixed_t* f); // decimal representation, caller must free

fixed_t* fixed_rescale(const fixed_t* f, int new_frac_bits);

fixed_t* fixed_add(const fixed_t* a, const fixed_t* b);
fixed_t* fixed_sub(const fixed_t* a, const fixed_t* b);
fixed_t* fixed_mul(const fixed_t* a, const fixed_t* b);
fixed_t* fixed_div(const fixed_t* a, const fixed_t* b);

int fixed_add_assign(fixed_t* a, const fixed_t* b);
int fixed_sub_assign(fixed_t* a, const fixed_t* b);
int fixed_mul_assign(fixed_t* a, const fixed_t* b);
int fixed_div_assign(fixed_t* a, const fixed_t* b);

int fixed_cmp(const fixed_t* a, const fixed_t* b);

#ifdef __cplusplus
}
#endif

#ifdef TZFIXED_IMPLEMENTATION

#ifndef fixed_assert
#  include <assert.h>
#  define fixed_assert(expr) assert(expr)
#endif
#ifndef fixed_alloc
#  include <stdlib.h>
#  define fixed_alloc(size) malloc(size)
#  define fixed_free(ptr)   free(ptr)
#endif

#include <string.h>
#include <math.h>

struct fixed_t {
    bigint_t* num;
    int frac_bits;
};

fixed_t* fixed_create(int frac_bits)
{
    fixed_t* f = (fixed_t*)fixed_alloc(sizeof(fixed_t));
    if (!f) return NULL;
    f->num = bigint_create();
    if (!f->num) { fixed_free(f); return NULL; }
    f->frac_bits = frac_bits;
    return f;
}

fixed_t* fixed_from_double(double value, int frac_bits)
{
    fixed_t* f = fixed_create(frac_bits);
    if (!f) return NULL;

    double scale  = pow(2.0, frac_bits);
    int64_t scaled = (int64_t)(value * scale + (value < 0 ? -0.5 : 0.5));

    bigint_destroy(f->num);
    f->num = bigint_from_int64(scaled);
    if (!f->num) { fixed_free(f); return NULL; }
    return f;
}

fixed_t* fixed_from_string(const char* str, int frac_bits)
{
    fixed_t* f = fixed_create(frac_bits);
    if (!f) return NULL;

    while (*str == ' ' || *str == '\t') str++;
    int neg = 0;
    if      (*str == '-') { neg = 1; str++; }
    else if (*str == '+') { str++; }

    const char* dot = strchr(str, '.');
    size_t int_len  = dot ? (size_t)(dot - str) : strlen(str);
    size_t frac_len = dot ? strlen(dot + 1) : 0;

    char* int_str = (char*)fixed_alloc(int_len + 1);
    if (!int_str) { fixed_destroy(f); return NULL; }
    memcpy(int_str, str, int_len);
    int_str[int_len] = '\0';

    bigint_t* integer = bigint_from_string(int_str);
    fixed_free(int_str);
    if (!integer) { fixed_destroy(f); return NULL; }

    bigint_shl(integer, frac_bits);

    if (frac_len > 0) {
        char* frac_str = (char*)fixed_alloc(frac_len + 1);
        if (!frac_str) { bigint_destroy(integer); fixed_destroy(f); return NULL; }
        memcpy(frac_str, dot + 1, frac_len);
        frac_str[frac_len] = '\0';

        double frac_val = atof(frac_str);
        fixed_free(frac_str);

        double scale    = pow(2.0, frac_bits) / pow(10.0, (double)frac_len);
        int64_t scaled  = (int64_t)(frac_val * scale + 0.5);

        bigint_t* frac_bigint = bigint_from_int64(scaled);
        if (!frac_bigint) { bigint_destroy(integer); fixed_destroy(f); return NULL; }
        bigint_add_assign(integer, frac_bigint);
        bigint_destroy(frac_bigint);
    }

    if (neg) bigint_set_negative(integer, 1);

    bigint_destroy(f->num);
    f->num = integer;
    return f;
}

fixed_t* fixed_clone(const fixed_t* f)
{
    fixed_t* c = fixed_create(f->frac_bits);
    if (!c) return NULL;
    bigint_destroy(c->num);
    c->num = bigint_clone(f->num);
    if (!c->num) { fixed_free(c); return NULL; }
    return c;
}

void fixed_destroy(fixed_t* f)
{
    if (!f) return;
    if (f->num) bigint_destroy(f->num);
    fixed_free(f);
}

int      fixed_get_frac_bits(const fixed_t* f) { return f->frac_bits; }
bigint_t* fixed_get_numerator(const fixed_t* f)  { return f->num; }

double fixed_to_double(const fixed_t* f)
{
    return bigint_to_double(f->num) / pow(2.0, f->frac_bits);
}

char* fixed_to_string(const fixed_t* f)
{
    char* result = (char*)fixed_alloc(32);
    if (result) snprintf(result, 32, "%f", fixed_to_double(f));
    return result;
}

fixed_t* fixed_rescale(const fixed_t* f, int new_frac_bits)
{
    fixed_t* result = fixed_create(new_frac_bits);
    if (!result) return NULL;
    bigint_destroy(result->num);
    result->num = bigint_clone(f->num);
    if (!result->num) { fixed_destroy(result); return NULL; }

    int diff = new_frac_bits - f->frac_bits;
    if      (diff > 0) bigint_shl(result->num,  (size_t)diff);
    else if (diff < 0) bigint_shr(result->num, (size_t)-diff);
    return result;
}

int fixed_add_assign(fixed_t* a, const fixed_t* b)
{
    if (a->frac_bits != b->frac_bits) return -1;
    return bigint_add_assign(a->num, b->num);
}

int fixed_sub_assign(fixed_t* a, const fixed_t* b)
{
    if (a->frac_bits != b->frac_bits) return -1;
    return bigint_sub_assign(a->num, b->num);
}

int fixed_mul_assign(fixed_t* a, const fixed_t* b)
{
    bigint_mul_assign(a->num, b->num);
    bigint_shr(a->num, (size_t)b->frac_bits);
    return 0;
}

int fixed_div_assign(fixed_t* a, const fixed_t* b)
{
    bigint_shl(a->num, (size_t)b->frac_bits);
    return bigint_div_assign(a->num, b->num);
}

fixed_t* fixed_add(const fixed_t* a, const fixed_t* b)
{
    fixed_t* r = fixed_clone(a);
    if (!r) return NULL;
    if (fixed_add_assign(r, b) != 0) { fixed_destroy(r); return NULL; }
    return r;
}

fixed_t* fixed_sub(const fixed_t* a, const fixed_t* b)
{
    fixed_t* r = fixed_clone(a);
    if (!r) return NULL;
    if (fixed_sub_assign(r, b) != 0) { fixed_destroy(r); return NULL; }
    return r;
}

fixed_t* fixed_mul(const fixed_t* a, const fixed_t* b)
{
    fixed_t* r = fixed_clone(a);
    if (!r) return NULL;
    if (fixed_mul_assign(r, b) != 0) { fixed_destroy(r); return NULL; }
    return r;
}

fixed_t* fixed_div(const fixed_t* a, const fixed_t* b)
{
    fixed_t* r = fixed_clone(a);
    if (!r) return NULL;
    if (fixed_div_assign(r, b) != 0) { fixed_destroy(r); return NULL; }
    return r;
}

int fixed_cmp(const fixed_t* a, const fixed_t* b)
{
    if (a->frac_bits == b->frac_bits)
        return bigint_cmp(a->num, b->num);

    // Downscale the higher-precision value to avoid precision drift.
    if (a->frac_bits > b->frac_bits) {
        fixed_t* as = fixed_rescale(a, b->frac_bits);
        if (!as) return 0;
        int r = bigint_cmp(as->num, b->num);
        fixed_destroy(as);
        return r;
    } else {
        fixed_t* bs = fixed_rescale(b, a->frac_bits);
        if (!bs) return 0;
        int r = bigint_cmp(a->num, bs->num);
        fixed_destroy(bs);
        return r;
    }
}

#endif // TZFIXED_IMPLEMENTATION

#endif // TZFIXED_H

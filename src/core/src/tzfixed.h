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

int fixed_get_frac_bits(const fixed_t* f);
bigint_t* fixed_get_numerator(const fixed_t* f); // borrowed reference, do not free

double fixed_to_double(const fixed_t* f);
char* fixed_to_string(const fixed_t* f); // decimal representation, caller must free

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

fixed_t* fixed_sqrt(const fixed_t* a);
fixed_t* fixed_sin(const fixed_t* a);
fixed_t* fixed_cos(const fixed_t* a);
fixed_t* fixed_atan2(const fixed_t* y, const fixed_t* x);

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
#include <stdio.h>

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

/* ── Taylor-series helpers (argument already in [0, π/4]) ───────────────── */

/* sin(x) = x - x³/3! + x⁵/5! - …   term_k = term_{k-1} * (-x²) / (2k·(2k+1)) */
static fixed_t* fixed_sin_core(const fixed_t* x)
{
    int fb = x->frac_bits;

    fixed_t* x2 = fixed_mul(x, x);
    if (!x2) return NULL;
    /* neg_x2 = −x² */
    fixed_t* neg_x2 = fixed_clone(x2);
    fixed_destroy(x2);
    if (!neg_x2) return NULL;
    if (!bigint_is_zero(neg_x2->num))
        bigint_set_negative(neg_x2->num, 1);

    fixed_t* sum  = fixed_clone(x);
    fixed_t* term = fixed_clone(x);
    if (!sum || !term) {
        fixed_destroy(neg_x2); fixed_destroy(sum); fixed_destroy(term);
        return NULL;
    }

    for (int k = 1; k <= 12; ++k) {
        fixed_t* tmp = fixed_mul(term, neg_x2);
        fixed_destroy(term);
        if (!tmp) { fixed_destroy(neg_x2); fixed_destroy(sum); return NULL; }

        fixed_t* d = fixed_from_double((double)((2*k) * (2*k + 1)), fb);
        if (!d) { fixed_destroy(tmp); fixed_destroy(neg_x2); fixed_destroy(sum); return NULL; }

        term = fixed_div(tmp, d);
        fixed_destroy(tmp); fixed_destroy(d);
        if (!term) { fixed_destroy(neg_x2); fixed_destroy(sum); return NULL; }

        fixed_t* ns = fixed_add(sum, term);
        fixed_destroy(sum);
        if (!ns) { fixed_destroy(neg_x2); fixed_destroy(term); return NULL; }
        sum = ns;
        if (bigint_is_zero(term->num)) break;
    }

    fixed_destroy(term); fixed_destroy(neg_x2);
    return sum;
}

/* cos(x) = 1 - x²/2! + x⁴/4! - …  term_k = term_{k-1} * (-x²) / ((2k-1)·2k) */
static fixed_t* fixed_cos_core(const fixed_t* x)
{
    int fb = x->frac_bits;

    fixed_t* x2 = fixed_mul(x, x);
    if (!x2) return NULL;
    fixed_t* neg_x2 = fixed_clone(x2);
    fixed_destroy(x2);
    if (!neg_x2) return NULL;
    if (!bigint_is_zero(neg_x2->num))
        bigint_set_negative(neg_x2->num, 1);

    fixed_t* sum  = fixed_from_double(1.0, fb);
    fixed_t* term = fixed_from_double(1.0, fb);
    if (!sum || !term) {
        fixed_destroy(neg_x2); fixed_destroy(sum); fixed_destroy(term);
        return NULL;
    }

    for (int k = 1; k <= 12; ++k) {
        fixed_t* tmp = fixed_mul(term, neg_x2);
        fixed_destroy(term);
        if (!tmp) { fixed_destroy(neg_x2); fixed_destroy(sum); return NULL; }

        fixed_t* d = fixed_from_double((double)((2*k - 1) * (2*k)), fb);
        if (!d) { fixed_destroy(tmp); fixed_destroy(neg_x2); fixed_destroy(sum); return NULL; }

        term = fixed_div(tmp, d);
        fixed_destroy(tmp); fixed_destroy(d);
        if (!term) { fixed_destroy(neg_x2); fixed_destroy(sum); return NULL; }

        fixed_t* ns = fixed_add(sum, term);
        fixed_destroy(sum);
        if (!ns) { fixed_destroy(neg_x2); fixed_destroy(term); return NULL; }
        sum = ns;
        if (bigint_is_zero(term->num)) break;
    }

    fixed_destroy(term); fixed_destroy(neg_x2);
    return sum;
}

/* atan(t) Taylor for small |t|: t - t³/3 + t⁵/5 - …
   term_k = term_{k-1} * (-t²) * (2k-1) / (2k+1) */
static fixed_t* fixed_atan_core(const fixed_t* t)
{
    int fb = t->frac_bits;

    fixed_t* t2 = fixed_mul(t, t);
    if (!t2) return NULL;
    fixed_t* neg_t2 = fixed_clone(t2);
    fixed_destroy(t2);
    if (!neg_t2) return NULL;
    if (!bigint_is_zero(neg_t2->num))
        bigint_set_negative(neg_t2->num, 1);

    fixed_t* sum  = fixed_clone(t);
    fixed_t* term = fixed_clone(t);
    if (!sum || !term) {
        fixed_destroy(neg_t2); fixed_destroy(sum); fixed_destroy(term);
        return NULL;
    }

    for (int k = 1; k <= 20; ++k) {
        /* term *= (-t²) */
        fixed_t* tmp = fixed_mul(term, neg_t2);
        fixed_destroy(term);
        if (!tmp) { fixed_destroy(neg_t2); fixed_destroy(sum); return NULL; }

        /* term *= (2k-1) */
        fixed_t* nf = fixed_from_double((double)(2*k - 1), fb);
        if (!nf) { fixed_destroy(tmp); fixed_destroy(neg_t2); fixed_destroy(sum); return NULL; }
        fixed_t* tmp2 = fixed_mul(tmp, nf);
        fixed_destroy(tmp); fixed_destroy(nf);
        if (!tmp2) { fixed_destroy(neg_t2); fixed_destroy(sum); return NULL; }

        /* term /= (2k+1) */
        fixed_t* df = fixed_from_double((double)(2*k + 1), fb);
        if (!df) { fixed_destroy(tmp2); fixed_destroy(neg_t2); fixed_destroy(sum); return NULL; }
        term = fixed_div(tmp2, df);
        fixed_destroy(tmp2); fixed_destroy(df);
        if (!term) { fixed_destroy(neg_t2); fixed_destroy(sum); return NULL; }

        fixed_t* ns = fixed_add(sum, term);
        fixed_destroy(sum);
        if (!ns) { fixed_destroy(neg_t2); fixed_destroy(term); return NULL; }
        sum = ns;
        if (bigint_is_zero(term->num)) break;
    }

    fixed_destroy(term); fixed_destroy(neg_t2);
    return sum;
}

/* ── Public math functions ───────────────────────────────────────────────── */

fixed_t* fixed_sqrt(const fixed_t* a)
{
    if (bigint_is_negative(a->num)) return NULL;
    if (bigint_is_zero(a->num))     return fixed_clone(a);

    int fb = a->frac_bits;
    fixed_t* x = fixed_from_double(sqrt(fixed_to_double(a)), fb);
    if (!x) return NULL;

    fixed_t* two = fixed_from_double(2.0, fb);
    if (!two) { fixed_destroy(x); return NULL; }

    for (int iter = 0; iter < 64; ++iter) {
        fixed_t* ax  = fixed_div(a, x);
        if (!ax) { fixed_destroy(x); fixed_destroy(two); return NULL; }

        fixed_t* sum = fixed_add(x, ax);
        fixed_destroy(ax);
        if (!sum) { fixed_destroy(x); fixed_destroy(two); return NULL; }

        fixed_t* xn = fixed_div(sum, two);
        fixed_destroy(sum);
        if (!xn) { fixed_destroy(x); fixed_destroy(two); return NULL; }

        fixed_t* diff = fixed_sub(xn, x);
        int done = 0;
        if (diff) {
            bigint_t* one_bi = bigint_from_int64(1);
            if (one_bi) {
                done = (bigint_cmp_abs(diff->num, one_bi) <= 0);
                bigint_destroy(one_bi);
            }
            fixed_destroy(diff);
        }
        fixed_destroy(x);
        x = xn;
        if (done) break;
    }

    fixed_destroy(two);
    return x;
}

fixed_t* fixed_sin(const fixed_t* a)
{
    int fb = a->frac_bits;

    fixed_t* two_pi  = fixed_from_double(6.283185307179586, fb);
    fixed_t* pi      = fixed_from_double(3.141592653589793, fb);
    fixed_t* half_pi = fixed_from_double(1.5707963267948966, fb);
    fixed_t* qtr_pi  = fixed_from_double(0.7853981633974483, fb);
    if (!two_pi || !pi || !half_pi || !qtr_pi) {
        fixed_destroy(two_pi); fixed_destroy(pi);
        fixed_destroy(half_pi); fixed_destroy(qtr_pi);
        return NULL;
    }

    int neg_result = bigint_is_negative(a->num);
    fixed_t* r = fixed_clone(a);
    if (!r) {
        fixed_destroy(two_pi); fixed_destroy(pi);
        fixed_destroy(half_pi); fixed_destroy(qtr_pi);
        return NULL;
    }
    bigint_set_negative(r->num, 0);           /* r = |a| */

    bigint_mod_assign(r->num, two_pi->num);   /* r ∈ [0, 2π) */
    fixed_destroy(two_pi);

    /* [π, 2π) → [0, π), flip sign */
    if (fixed_cmp(r, pi) >= 0) {
        fixed_sub_assign(r, pi);
        neg_result = !neg_result;
    }

    /* [π/2, π) → [0, π/2] via sin(x) = sin(π − x) */
    if (fixed_cmp(r, half_pi) > 0) {
        fixed_t* tmp = fixed_sub(pi, r);
        fixed_destroy(r);
        if (!tmp) {
            fixed_destroy(pi); fixed_destroy(half_pi); fixed_destroy(qtr_pi);
            return NULL;
        }
        r = tmp;
    }

    /* [π/4, π/2] → [0, π/4] via sin(x) = cos(π/2 − x) */
    fixed_t* result;
    if (fixed_cmp(r, qtr_pi) > 0) {
        fixed_t* tmp = fixed_sub(half_pi, r);
        fixed_destroy(r);
        if (!tmp) {
            fixed_destroy(pi); fixed_destroy(half_pi); fixed_destroy(qtr_pi);
            return NULL;
        }
        result = fixed_cos_core(tmp);
        fixed_destroy(tmp);
    } else {
        result = fixed_sin_core(r);
        fixed_destroy(r);
    }

    fixed_destroy(pi); fixed_destroy(half_pi); fixed_destroy(qtr_pi);

    if (result && neg_result && !bigint_is_zero(result->num))
        bigint_set_negative(result->num, 1);
    return result;
}

fixed_t* fixed_cos(const fixed_t* a)
{
    int fb = a->frac_bits;

    fixed_t* two_pi  = fixed_from_double(6.283185307179586, fb);
    fixed_t* pi      = fixed_from_double(3.141592653589793, fb);
    fixed_t* half_pi = fixed_from_double(1.5707963267948966, fb);
    fixed_t* qtr_pi  = fixed_from_double(0.7853981633974483, fb);
    if (!two_pi || !pi || !half_pi || !qtr_pi) {
        fixed_destroy(two_pi); fixed_destroy(pi);
        fixed_destroy(half_pi); fixed_destroy(qtr_pi);
        return NULL;
    }

    fixed_t* r = fixed_clone(a);
    if (!r) {
        fixed_destroy(two_pi); fixed_destroy(pi);
        fixed_destroy(half_pi); fixed_destroy(qtr_pi);
        return NULL;
    }
    bigint_set_negative(r->num, 0);           /* cos is even */

    bigint_mod_assign(r->num, two_pi->num);   /* r ∈ [0, 2π) */
    fixed_destroy(two_pi);

    /* [π, 2π) → [0, π) via cos(x) = −cos(x − π) */
    int neg_result = 0;
    if (fixed_cmp(r, pi) >= 0) {
        fixed_sub_assign(r, pi);
        neg_result = 1;
    }

    /* [π/2, π) → [0, π/2] via cos(x) = −cos(π − x) */
    if (fixed_cmp(r, half_pi) > 0) {
        fixed_t* tmp = fixed_sub(pi, r);
        fixed_destroy(r);
        if (!tmp) {
            fixed_destroy(pi); fixed_destroy(half_pi); fixed_destroy(qtr_pi);
            return NULL;
        }
        r = tmp;
        neg_result = !neg_result;
    }

    /* [π/4, π/2] → [0, π/4] via cos(x) = sin(π/2 − x) */
    fixed_t* result;
    if (fixed_cmp(r, qtr_pi) > 0) {
        fixed_t* tmp = fixed_sub(half_pi, r);
        fixed_destroy(r);
        if (!tmp) {
            fixed_destroy(pi); fixed_destroy(half_pi); fixed_destroy(qtr_pi);
            return NULL;
        }
        result = fixed_sin_core(tmp);
        fixed_destroy(tmp);
    } else {
        result = fixed_cos_core(r);
        fixed_destroy(r);
    }

    fixed_destroy(pi); fixed_destroy(half_pi); fixed_destroy(qtr_pi);

    if (result && neg_result && !bigint_is_zero(result->num))
        bigint_set_negative(result->num, 1);
    return result;
}

fixed_t* fixed_atan2(const fixed_t* y, const fixed_t* x)
{
    int fb = y->frac_bits;

    fixed_t* one     = fixed_from_double(1.0, fb);
    fixed_t* pi      = fixed_from_double(3.141592653589793, fb);
    fixed_t* half_pi = fixed_from_double(1.5707963267948966, fb);
    if (!one || !pi || !half_pi) {
        fixed_destroy(one); fixed_destroy(pi); fixed_destroy(half_pi);
        return NULL;
    }

    int x_neg  = bigint_is_negative(x->num);
    int y_neg  = bigint_is_negative(y->num);
    int x_zero = bigint_is_zero(x->num);

    fixed_t* result = NULL;

    /* Special case: x = 0 */
    if (x_zero) {
        result = fixed_clone(half_pi);
        if (result && y_neg && !bigint_is_zero(result->num))
            bigint_set_negative(result->num, 1);
        goto atan2_done;
    }

    {
        /* Compute |y|/|x| */
        fixed_t* ax = fixed_clone(x);
        fixed_t* ay = fixed_clone(y);
        if (!ax || !ay) { fixed_destroy(ax); fixed_destroy(ay); goto atan2_done; }
        bigint_set_negative(ax->num, 0);
        bigint_set_negative(ay->num, 0);

        fixed_t* t = fixed_div(ay, ax);
        fixed_destroy(ay); fixed_destroy(ax);
        if (!t) goto atan2_done;

        /* Half-angle reduction: 6 iterations t → t/(1+√(1+t²)) */
        for (int i = 0; i < 6; ++i) {
            fixed_t* t2    = fixed_mul(t, t);
            if (!t2) { fixed_destroy(t); t = NULL; break; }

            fixed_t* one_t2 = fixed_add(one, t2);
            fixed_destroy(t2);
            if (!one_t2) { fixed_destroy(t); t = NULL; break; }

            fixed_t* sq = fixed_sqrt(one_t2);
            fixed_destroy(one_t2);
            if (!sq) { fixed_destroy(t); t = NULL; break; }

            fixed_t* denom = fixed_add(one, sq);
            fixed_destroy(sq);
            if (!denom) { fixed_destroy(t); t = NULL; break; }

            fixed_t* nt = fixed_div(t, denom);
            fixed_destroy(denom); fixed_destroy(t);
            t = nt;
            if (!t) break;
        }
        if (!t) goto atan2_done;

        /* atan(t_reduced) × 2^6 */
        fixed_t* at = fixed_atan_core(t);
        fixed_destroy(t);
        if (!at) goto atan2_done;

        fixed_t* scale = fixed_from_double(64.0, fb);
        if (!scale) { fixed_destroy(at); goto atan2_done; }
        fixed_t* angle = fixed_mul(at, scale);
        fixed_destroy(at); fixed_destroy(scale);
        if (!angle) goto atan2_done;

        /* Quadrant correction (angle is atan(|y|/|x|) ∈ [0, π/2]) */
        if (x_neg) {
            /* Q2 / Q3: angle → π − angle */
            result = fixed_sub(pi, angle);
            fixed_destroy(angle);
        } else {
            result = angle;
        }

        /* Negate for y < 0 (Q3 / Q4) */
        if (result && y_neg && !bigint_is_zero(result->num))
            bigint_set_negative(result->num, 1);
    }

atan2_done:
    fixed_destroy(one); fixed_destroy(pi); fixed_destroy(half_pi);
    return result;
}

#endif // TZFIXED_IMPLEMENTATION

#endif // TZFIXED_H

// tzbigint_packed.h — packed-sign implementation for tzbigint.h
// Do not include directly. Define TZBIGINT_IMPLEMENTATION and include tzbigint.h.
//
// The sign is stored in the MSB (bit 31) of the most-significant limb.
// The magnitude of that limb therefore uses only bits 0-30. When the
// magnitude of the top limb would require bit 31, an extra zero limb is
// added to carry the sign, preserving the invariant:
//   (limbs[num_limbs-1] & 0x7FFFFFFF) == magnitude of top limb
//   (limbs[num_limbs-1] >> 31) == sign bit

#include <string.h>
#include <math.h>

typedef uint32_t bigint_limb_t;
#define BIGINT_LIMB_BITS 32
#define BIGINT_LIMB_MASK 0xFFFFFFFFu
#define BIGINT_SIGN_BIT 0x80000000u
#define BIGINT_MAGN_MASK 0x7FFFFFFFu

struct bigint_t
{
    size_t num_limbs;
    bigint_limb_t* limbs; // sign in bit 31 of limbs[num_limbs-1]
};

static int bigint_packed_neg(const bigint_t* bi)
{
    return (bi->limbs[bi->num_limbs - 1] & BIGINT_SIGN_BIT) ? 1 : 0;
}

static bigint_limb_t bigint_packed_limb(const bigint_t* bi, size_t i)
{
    bigint_limb_t v = bi->limbs[i];
    if (i == bi->num_limbs - 1)
        v &= BIGINT_MAGN_MASK;
    return v;
}

static int bigint_packed_is_zero(const bigint_t* bi)
{
    for (size_t i = 0; i < bi->num_limbs; i++)
        if (bigint_packed_limb(bi, i))
            return 0;
    return 1;
}

static int bigint_packed_grow_for_sign(bigint_t* bi)
{
    if (!(bi->limbs[bi->num_limbs - 1] & BIGINT_SIGN_BIT))
        return 0;
    size_t nn = bi->num_limbs + 1;
    bigint_limb_t* nl = (bigint_limb_t*)bigint_alloc(nn * sizeof(bigint_limb_t));
    if (!nl)
        return -1;
    memcpy(nl, bi->limbs, bi->num_limbs * sizeof(bigint_limb_t));
    nl[bi->num_limbs] = 0;
    bigint_free(bi->limbs);
    bi->limbs = nl; bi->num_limbs = nn;
    return 0;
}

static void bigint_packed_normalize(bigint_t* bi)
{
    int neg = bigint_packed_neg(bi);
    bi->limbs[bi->num_limbs - 1] &= BIGINT_MAGN_MASK; // clear sign

    while (bi->num_limbs > 1 && bi->limbs[bi->num_limbs - 1] == 0)
        bi->num_limbs--;

    bigint_packed_grow_for_sign(bi);

    if (bigint_packed_is_zero(bi)) {
        bi->num_limbs = 1;
        bi->limbs[0] = 0;
        return;
    }
    if (neg)
        bi->limbs[bi->num_limbs - 1] |= BIGINT_SIGN_BIT;
}

static void bigint_packed_normalize_mag(bigint_t* bi)
{
    while (bi->num_limbs > 1 && bi->limbs[bi->num_limbs - 1] == 0)
        bi->num_limbs--;
    bigint_packed_grow_for_sign(bi);
}

static int bigint_packed_cmp_mag(const bigint_t* a, const bigint_t* b)
{
    if (a->num_limbs != b->num_limbs)
        return (a->num_limbs < b->num_limbs) ? -1 : 1;
    for (size_t i = a->num_limbs; i-- > 0; ) {
        bigint_limb_t la = bigint_packed_limb(a, i), lb = bigint_packed_limb(b, i);
        if (la != lb) 
            return (la < lb) ? -1 : 1;
    }
    return 0;
}

static int bigint_packed_add_mag(bigint_t* a, const bigint_t* b)
{
    size_t na = a->num_limbs, nb = b->num_limbs;
    size_t n  = (na > nb ? na : nb) + 1;

    bigint_limb_t* nl = (bigint_limb_t*)bigint_alloc(n * sizeof(bigint_limb_t));
    if (!nl)
        return -1;
    memset(nl, 0, n * sizeof(bigint_limb_t));

    uint64_t carry = 0;
    for (size_t i = 0; i < n; i++) {
        uint64_t la = (i < na) ? (uint64_t)bigint_packed_limb(a, i) : 0;
        uint64_t lb = (i < nb) ? (uint64_t)bigint_packed_limb(b, i) : 0;
        uint64_t s  = la + lb + carry;
        nl[i] = (bigint_limb_t)(s & BIGINT_LIMB_MASK);
        carry = s >> BIGINT_LIMB_BITS;
    }

    bigint_free(a->limbs);
    a->limbs = nl; a->num_limbs = n;
    bigint_packed_normalize_mag(a);
    return 0;
}

static int bigint_packed_sub_mag(bigint_t* a, const bigint_t* b)
{
    size_t na = a->num_limbs, nb = b->num_limbs;
    int64_t borrow = 0;
    for (size_t i = 0; i < na; i++) {
        int64_t la = (int64_t)bigint_packed_limb(a, i);
        int64_t lb = (i < nb) ? (int64_t)bigint_packed_limb(b, i) : 0;
        int64_t d  = la - lb - borrow;
        if (d < 0) { d += (int64_t)0x100000000LL; borrow = 1; }
        else borrow = 0;
        a->limbs[i] = (bigint_limb_t)d;
    }
    bigint_packed_normalize_mag(a);
    return 0;
}

static size_t bigint_packed_bit_len(const bigint_t* bi)
{
    for (size_t i = bi->num_limbs; i-- > 0; ) {
        bigint_limb_t v = bigint_packed_limb(bi, i);
        if (v) {
            size_t bits = i * BIGINT_LIMB_BITS;
            while (v) { bits++; v >>= 1; }
            return bits;
        }
    }
    return 0;
}

static int bigint_packed_get_bit(const bigint_t* bi, size_t pos)
{
    size_t li = pos / BIGINT_LIMB_BITS, bi2 = pos % BIGINT_LIMB_BITS;
    if (li >= bi->num_limbs)
        return 0;
    return (bigint_packed_limb(bi, li) >> bi2) & 1;
}

bigint_t* bigint_create(void)
{
    bigint_t* bi = (bigint_t*)bigint_alloc(sizeof(bigint_t));
    if (!bi)
        return NULL;
    bi->num_limbs = 1;
    bi->limbs = (bigint_limb_t*)bigint_alloc(sizeof(bigint_limb_t));
    if (!bi->limbs) {
        bigint_free(bi);
        return NULL;
    }
    bi->limbs[0] = 0;
    return bi;
}

bigint_t* bigint_from_int64(int64_t value)
{
    bigint_t* bi = bigint_create();
    if (!bi)
        return NULL;

    int neg = (value < 0) ? 1 : 0;
    uint64_t abs_val = neg ? -(uint64_t)value : (uint64_t)value;

    size_t n = 0;
    uint64_t tmp = abs_val;
    do { n++; tmp >>= BIGINT_LIMB_BITS; } while (tmp);

    bigint_free(bi->limbs);
    bi->limbs = (bigint_limb_t*)bigint_alloc(n * sizeof(bigint_limb_t));
    if (!bi->limbs) {
        bigint_free(bi);
        return NULL;
    }
    bi->num_limbs = n;
    for (size_t i = 0; i < n; i++) {
        bi->limbs[i] = (bigint_limb_t)(abs_val & BIGINT_LIMB_MASK);
        abs_val >>= BIGINT_LIMB_BITS;
    }

    if (bi->limbs[n - 1] & BIGINT_SIGN_BIT) {
        bigint_limb_t* nl = (bigint_limb_t*)bigint_alloc((n + 1) * sizeof(bigint_limb_t));
        if (!nl) {
            bigint_destroy(bi);
            return NULL;
        }
        memcpy(nl, bi->limbs, n * sizeof(bigint_limb_t));
        nl[n] = 0;
        bigint_free(bi->limbs);
        bi->limbs = nl; bi->num_limbs = n + 1;
    }

    if (neg && !bigint_packed_is_zero(bi))
        bi->limbs[bi->num_limbs - 1] |= BIGINT_SIGN_BIT;
    return bi;
}

bigint_t* bigint_from_string(const char* str)
{
    bigint_t* bi = bigint_create();
    if (!bi)
        return NULL;

    while (*str == ' ' || *str == '\t') str++;
    int neg = 0;
    if (*str == '-') { neg = 1; str++; }
    else if (*str == '+') { str++; }

    bigint_t* ten = bigint_from_int64(10);
    bigint_t* digit = bigint_from_int64(0);
    if (!ten || !digit) {
        bigint_destroy(ten);
        bigint_destroy(digit);
        bigint_destroy(bi);
        return NULL;
    }

    while (*str >= '0' && *str <= '9') {
        bigint_mul_assign(bi, ten);
        bigint_destroy(digit);
        digit = bigint_from_int64(*str - '0');
        if (!digit) {
            bigint_destroy(ten);
            bigint_destroy(bi);
            return NULL;
        }
        bigint_add_assign(bi, digit);
        str++;
    }

    bigint_destroy(ten);
    bigint_destroy(digit);
    if (neg)
        bigint_set_negative(bi, 1);
    return bi;
}

bigint_t* bigint_clone(const bigint_t* bi)
{
    bigint_t* c = bigint_create();
    if (!c)
        return NULL;
    bigint_free(c->limbs);
    c->limbs = (bigint_limb_t*)bigint_alloc(bi->num_limbs * sizeof(bigint_limb_t));
    if (!c->limbs) {
        bigint_destroy(c);
        return NULL;
    }
    c->num_limbs = bi->num_limbs;
    memcpy(c->limbs, bi->limbs, bi->num_limbs * sizeof(bigint_limb_t));
    return c;
}

void bigint_destroy(bigint_t* bi)
{
    if (!bi)
        return;
    bigint_free(bi->limbs);
    bigint_free(bi);
}

int bigint_is_zero(const bigint_t* bi)
{
    return bigint_packed_is_zero(bi);
}
int bigint_is_negative(const bigint_t* bi)
{
    return bigint_packed_is_zero(bi) ? 0 : bigint_packed_neg(bi);
}

void bigint_set_negative(bigint_t* bi, int neg)
{
    if (bigint_packed_is_zero(bi))
        return;
    if (neg) bi->limbs[bi->num_limbs - 1] |= BIGINT_SIGN_BIT;
    else bi->limbs[bi->num_limbs - 1] &= BIGINT_MAGN_MASK;
}

int bigint_cmp_abs(const bigint_t* a, const bigint_t* b)
{
    return bigint_packed_cmp_mag(a, b);
}

int bigint_cmp(const bigint_t* a, const bigint_t* b)
{
    if (bigint_packed_is_zero(a) && bigint_packed_is_zero(b))
        return 0;
    int na = bigint_is_negative(a), nb = bigint_is_negative(b);
    if (na != nb)
        return na ? -1 : 1;
    int cmp = bigint_packed_cmp_mag(a, b);
    return na ? -cmp : cmp;
}

int bigint_add_assign(bigint_t* a, const bigint_t* b)
{
    int neg_a = bigint_packed_neg(a);
    int neg_b = bigint_packed_neg(b);
    a->limbs[a->num_limbs - 1] &= BIGINT_MAGN_MASK; // clear a's sign for arithmetic

    if (neg_a == neg_b) {
        if (bigint_packed_add_mag(a, b) < 0)
            return -1;
        if (neg_a && !bigint_packed_is_zero(a))
            a->limbs[a->num_limbs - 1] |= BIGINT_SIGN_BIT;
        return 0;
    }

    int cmp = bigint_packed_cmp_mag(a, b);
    if (cmp == 0) {
        a->num_limbs = 1; a->limbs[0] = 0;
        return 0;
    } else if (cmp > 0) {
        if (bigint_packed_sub_mag(a, b) < 0)
            return -1;
        if (neg_a && !bigint_packed_is_zero(a))
            a->limbs[a->num_limbs - 1] |= BIGINT_SIGN_BIT;
    } else {
        bigint_t* tmp = bigint_clone(b);
        if (!tmp) {
            if (neg_a)
                a->limbs[a->num_limbs-1] |= BIGINT_SIGN_BIT;
            return -1;
        }
        // clear b's sign in tmp so bigint_packed_sub_mag sees a clean magnitude
        tmp->limbs[tmp->num_limbs - 1] &= BIGINT_MAGN_MASK;
        if (bigint_packed_sub_mag(tmp, a) < 0) {
            bigint_destroy(tmp);
            return -1;
        }
        bigint_free(a->limbs);
        a->limbs = tmp->limbs; a->num_limbs = tmp->num_limbs;
        tmp->limbs = NULL;
        bigint_destroy(tmp);
        if (neg_b && !bigint_packed_is_zero(a))
            a->limbs[a->num_limbs - 1] |= BIGINT_SIGN_BIT;
    }
    return 0;
}

int bigint_sub_assign(bigint_t* a, const bigint_t* b)
{
    bigint_t* nb = bigint_clone(b);
    if (!nb)
        return -1;
    if (!bigint_packed_is_zero(nb))
        nb->limbs[nb->num_limbs - 1] ^= BIGINT_SIGN_BIT; // flip sign bit
    int r = bigint_add_assign(a, nb);
    bigint_destroy(nb);
    return r;
}

int bigint_mul_assign(bigint_t* a, const bigint_t* b)
{
    if (bigint_packed_is_zero(a) || bigint_packed_is_zero(b)) {
        a->num_limbs = 1; a->limbs[0] = 0;
        return 0;
    }
    int rn = (bigint_packed_neg(a) != bigint_packed_neg(b)) ? 1 : 0;

    size_t na = a->num_limbs, nb = b->num_limbs, nc = na + nb;
    bigint_limb_t* c = (bigint_limb_t*)bigint_alloc(nc * sizeof(bigint_limb_t));
    if (!c)
        return -1;
    memset(c, 0, nc * sizeof(bigint_limb_t));

    for (size_t i = 0; i < na; i++) {
        uint64_t carry = 0;
        for (size_t j = 0; j < nb; j++) {
            uint64_t p = (uint64_t)bigint_packed_limb(a, i) * (uint64_t)bigint_packed_limb(b, j) + c[i+j] + carry;
            c[i+j] = (bigint_limb_t)(p & BIGINT_LIMB_MASK);
            carry = p >> BIGINT_LIMB_BITS;
        }
        if (carry)
            c[i+nb] = (bigint_limb_t)((uint64_t)c[i+nb] + carry);
    }

    bigint_free(a->limbs);
    a->limbs = c; a->num_limbs = nc;
    bigint_packed_normalize_mag(a); // strips zeros, ensures sign-bit room
    if (rn && !bigint_packed_is_zero(a))
        a->limbs[a->num_limbs - 1] |= BIGINT_SIGN_BIT;
    return 0;
}

static bigint_t* bigint_packed_divmod(bigint_t* a, const bigint_t* b)
{
    bigint_t* q = bigint_from_int64(0);
    bigint_t* r = bigint_from_int64(0);
    if (!q || !r) {
        bigint_destroy(q);
        bigint_destroy(r);
        return NULL;
    }

    size_t bits = bigint_packed_bit_len(a);
    for (size_t i = bits; i-- > 0; ) {
        bigint_add_assign(r, r); // r <<= 1 (goes through packed add_assign)
        if (bigint_packed_get_bit(a, i)) r->limbs[0] |= 1;
        if (bigint_packed_cmp_mag(r, b) >= 0) {
            // clear r's sign before sub (r is non-negative in this context)
            r->limbs[r->num_limbs - 1] &= BIGINT_MAGN_MASK;
            bigint_packed_sub_mag(r, b);
            size_t li = i / BIGINT_LIMB_BITS, bi2 = i % BIGINT_LIMB_BITS;
            if (li >= q->num_limbs) {
                size_t nn = li + 1;
                bigint_limb_t* nl = (bigint_limb_t*)bigint_alloc(nn * sizeof(bigint_limb_t));
                if (!nl) { bigint_destroy(q); bigint_destroy(r); return NULL; }
                memset(nl, 0, nn * sizeof(bigint_limb_t));
                memcpy(nl, q->limbs, q->num_limbs * sizeof(bigint_limb_t));
                bigint_free(q->limbs);
                q->limbs = nl; q->num_limbs = nn;
            }
            q->limbs[li] |= (bigint_limb_t)1 << bi2;
        }
    }

    // Ensure q's top limb has sign-bit room
    bigint_packed_normalize_mag(q);

    bigint_free(a->limbs);
    a->limbs = q->limbs; a->num_limbs = q->num_limbs;
    q->limbs = NULL;
    bigint_destroy(q);
    return r;
}

int bigint_div_assign(bigint_t* a, const bigint_t* b)
{
    if (bigint_packed_is_zero(b)) return -1;
    int rn = (bigint_packed_neg(a) != bigint_packed_neg(b)) ? 1 : 0;
    a->limbs[a->num_limbs - 1] &= BIGINT_MAGN_MASK; // work on magnitude
    bigint_t* rem = bigint_packed_divmod(a, b);
    if (!rem)
        return -1;
    bigint_destroy(rem);
    if (rn && !bigint_packed_is_zero(a))
        a->limbs[a->num_limbs - 1] |= BIGINT_SIGN_BIT;
    return 0;
}

int bigint_mod_assign(bigint_t* a, const bigint_t* b)
{
    if (bigint_packed_is_zero(b))
        return -1;
    int an = bigint_packed_neg(a);
    a->limbs[a->num_limbs - 1] &= BIGINT_MAGN_MASK;
    bigint_t* rem = bigint_packed_divmod(a, b);
    if (!rem)
        return -1;
    bigint_free(a->limbs);
    a->limbs = rem->limbs; a->num_limbs = rem->num_limbs;
    rem->limbs = NULL;
    bigint_destroy(rem);
    if (an && !bigint_packed_is_zero(a))
        a->limbs[a->num_limbs - 1] |= BIGINT_SIGN_BIT;
    return 0;
}

void bigint_shl(bigint_t* bi, size_t bits)
{
    int neg = bigint_packed_neg(bi);
    bi->limbs[bi->num_limbs - 1] &= BIGINT_MAGN_MASK;

    size_t ls = bits / BIGINT_LIMB_BITS, bs = bits % BIGINT_LIMB_BITS;
    size_t ns = bi->num_limbs + ls + (bs ? 1 : 0);

    bigint_limb_t* nl = (bigint_limb_t*)bigint_alloc(ns * sizeof(bigint_limb_t));
    if (!nl)
        return;
    memset(nl, 0, ns * sizeof(bigint_limb_t));
    memcpy(nl + ls, bi->limbs, bi->num_limbs * sizeof(bigint_limb_t));

    if (bs) {
        bigint_limb_t carry = 0;
        for (size_t i = ls; i < ns; i++) {
            bigint_limb_t v = nl[i];
            nl[i]  = (v << bs) | carry;
            carry  = v >> (BIGINT_LIMB_BITS - bs);
        }
    }

    bigint_free(bi->limbs);
    bi->limbs = nl; bi->num_limbs = ns;
    bigint_packed_normalize(bi); // strips zeros, ensures sign-bit room, restores neg=0
    if (neg && !bigint_packed_is_zero(bi))
        bi->limbs[bi->num_limbs - 1] |= BIGINT_SIGN_BIT;
}

void bigint_shr(bigint_t* bi, size_t bits)
{
    int neg = bigint_packed_neg(bi);
    bi->limbs[bi->num_limbs - 1] &= BIGINT_MAGN_MASK;

    size_t ls = bits / BIGINT_LIMB_BITS, bs = bits % BIGINT_LIMB_BITS;
    if (ls >= bi->num_limbs) {
        bi->num_limbs = 1; bi->limbs[0] = 0;
        return;
    }
    size_t ns = bi->num_limbs - ls;
    for (size_t i = 0; i < ns; i++) bi->limbs[i] = bi->limbs[i + ls];
    if (bs) {
        for (size_t i = 0; i < ns - 1; i++)
            bi->limbs[i] = (bi->limbs[i] >> bs) | (bi->limbs[i+1] << (BIGINT_LIMB_BITS - bs));
        bi->limbs[ns - 1] >>= bs;
    }
    bi->num_limbs = ns;
    bigint_packed_normalize(bi);
    if (neg && !bigint_packed_is_zero(bi))
        bi->limbs[bi->num_limbs - 1] |= BIGINT_SIGN_BIT;
}

double bigint_to_double(const bigint_t* bi)
{
    double r = 0.0;
    for (size_t i = bi->num_limbs; i-- > 0; )
        r = r * 4294967296.0 + bigint_packed_limb(bi, i);
    return bigint_packed_neg(bi) ? -r : r;
}

char* bigint_to_string(const bigint_t* bi)
{
    if (bigint_packed_is_zero(bi)) {
        char* s = (char*)bigint_alloc(2);
        if (s) { s[0] = '0'; s[1] = '\0'; }
        return s;
    }

    bigint_t* tmp = bigint_clone(bi);
    if (!tmp)
        return NULL;
    tmp->limbs[tmp->num_limbs - 1] &= BIGINT_MAGN_MASK; // work on magnitude

    bigint_t* ten = bigint_from_int64(10);
    if (!ten) {
        bigint_destroy(tmp);
        return NULL;
    }

    size_t cap = 64;
    char* buf = (char*)bigint_alloc(cap);
    if (!buf) {
        bigint_destroy(tmp);
        bigint_destroy(ten);
        return NULL;
    }
    size_t len = 0;

    while (!bigint_packed_is_zero(tmp)) {
        bigint_t* rem = bigint_packed_divmod(tmp, ten);
        if (!rem) {
            bigint_free(buf);
            bigint_destroy(tmp);
            bigint_destroy(ten);
            return NULL;
        }
        int d = (int)bigint_packed_limb(rem, 0);
        bigint_destroy(rem);
        if (len >= cap - 1) {
            cap *= 2;
            char* nb = (char*)bigint_alloc(cap);
            if (!nb) {
                bigint_free(buf);
                bigint_destroy(tmp);
                bigint_destroy(ten);
                return NULL;
            }
            memcpy(nb, buf, len);
            bigint_free(buf);
            buf = nb;
        }
        buf[len++] = (char)('0' + d);
    }

    bigint_destroy(ten);
    bigint_destroy(tmp);

    int neg = bigint_is_negative(bi);
    char* result = (char*)bigint_alloc(len + (size_t)(neg ? 1 : 0) + 1);
    if (!result) {
        bigint_free(buf);
        return NULL;
    }
    size_t pos = 0;
    if (neg) result[pos++] = '-';
    for (size_t i = len; i-- > 0;)
        result[pos++] = buf[i];
    result[pos] = '\0';
    bigint_free(buf);
    return result;
}

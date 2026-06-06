#ifndef TZBIGINT_H
#define TZBIGINT_H

//
// tzbigint.h - Arbitrary precision integers
//
// Select implementation at compile time (before including):
//   #define TZBIGINT_IMPL TZBIGINT_IMPL_SEPARATE   (default: separate sign field)
//   #define TZBIGINT_IMPL TZBIGINT_IMPL_PACKED      (sign packed into MSB of top limb)
//
// Usage:
//   #define TZBIGINT_IMPLEMENTATION
//   #include "tzbigint.h"
//
// Custom allocator (optional, define before including):
//   #define bigint_assert(expr)
//   #define bigint_alloc(size)
//   #define bigint_free(ptr)
//

#include <stdint.h>
#include <stddef.h>

#define TZBIGINT_IMPL_SEPARATE 0
#define TZBIGINT_IMPL_PACKED   1

#ifndef TZBIGINT_IMPL
#  define TZBIGINT_IMPL TZBIGINT_IMPL_SEPARATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bigint_t bigint_t;

bigint_t* bigint_create(void);
bigint_t* bigint_from_int64(int64_t value);
bigint_t* bigint_from_string(const char* str);
bigint_t* bigint_clone(const bigint_t* bi);
void bigint_destroy(bigint_t* bi);

int bigint_is_zero(const bigint_t* bi);
int bigint_is_negative(const bigint_t* bi);
void bigint_set_negative(bigint_t* bi, int neg);

double bigint_to_double(const bigint_t* bi);
char* bigint_to_string(const bigint_t* bi); // caller must free

int bigint_add_assign(bigint_t* a, const bigint_t* b);
int bigint_sub_assign(bigint_t* a, const bigint_t* b);
int bigint_mul_assign(bigint_t* a, const bigint_t* b);
int bigint_div_assign(bigint_t* a, const bigint_t* b);
int bigint_mod_assign(bigint_t* a, const bigint_t* b);

void bigint_shl(bigint_t* bi, size_t bits);
void bigint_shr(bigint_t* bi, size_t bits);

int bigint_cmp(const bigint_t* a, const bigint_t* b);
int bigint_cmp_abs(const bigint_t* a, const bigint_t* b);

#ifdef __cplusplus
}
#endif

#ifdef TZBIGINT_IMPLEMENTATION

#ifndef bigint_assert
#  include <assert.h>
#  define bigint_assert(expr) assert(expr)
#endif
#ifndef bigint_alloc
#  include <stdlib.h>
#  define bigint_alloc(size) malloc(size)
#  define bigint_free(ptr) free(ptr)
#endif

#if TZBIGINT_IMPL == TZBIGINT_IMPL_SEPARATE
#  include "tzbigint_separate.h"
#elif TZBIGINT_IMPL == TZBIGINT_IMPL_PACKED
#  include "tzbigint_packed.h"
#else
#  error "Unknown TZBIGINT_IMPL: use TZBIGINT_IMPL_SEPARATE or TZBIGINT_IMPL_PACKED"
#endif

#endif // TZBIGINT_IMPLEMENTATION

#endif // TZBIGINT_H

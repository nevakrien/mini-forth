#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define TODO \
do { \
    fprintf(stderr, "TODO hit at %s:%d\n", __FILE__, __LINE__); \
    abort(); \
} while (0);

#if defined(__GNUC__) || defined(__clang__)
    #define COMPILER_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
    #define COMPILER_UNREACHABLE() __assume(0)
#else
    #define COMPILER_UNREACHABLE() ((void)0)
#endif


#ifndef NDEBUG
    #define ASSERT(x) assert(x)
    #define UNREACHABLE() assert(!"UNREACHABLE")
#else
    #define ASSERT(x) \
        do { if (!(x)) COMPILER_UNREACHABLE(); } while (0)

    #define UNREACHABLE() COMPILER_UNREACHABLE()
#endif

static inline void* xmalloc(size_t sz) {
    void* p = malloc(sz);
    if (!p) abort();
    return p;
}

static inline void* xrealloc(void* p, size_t sz) {
    p = realloc(p, sz);
    if (!p) abort();
    return p;
}

#define ARR_POP(arr) ((arr).data[(ASSERT(arr.len),--(arr).len)])
#define ARR_PEEK(arr) ((arr).data[(ASSERT(arr.len),(arr).len-1)])
#define ARR_AT(arr,i) ((arr).data[(ASSERT(((size_t)i)<((size_t)arr.len)),i)])
#define ARR_REMOVE_UNORDERED(arr,i) (AT(arr,i)=PEEK(arr),(arr).data[--(arr).len])

#define ARR_ENSURE_CAP(arr, need) \
({ \
    __auto_type _a = &(arr); \
    size_t _need = (size_t)(need); \
    if (_a->cap < _need) { \
        size_t _new = _a->cap ? _a->cap : 8; \
        while (_new < _need) _new *= 2; \
        _a->cap = _new; \
        _a->data = xrealloc(_a->data, _a->cap * sizeof(*_a->data)); \
    } \
})

#define ARR_PUSH(arr, x) \
(ARR_ENSURE_CAP(arr, (arr).len + 1), (arr).data[(arr).len] = (x), (arr).len++)


#endif // UTILS_H


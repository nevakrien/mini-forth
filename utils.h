#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

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

#define ARR_POP(arr) ((arr).data[(ASSERT(arr.len),--(arr).len)])
#define ARR_PEEK(arr) ((arr).data[(ASSERT(arr.len),(arr).len-1)])
#define ARR_AT(arr,i) ((arr).data[(ASSERT(((size_t)i)<((size_t)arr.len)),i)])
#define ARR_REMOVE_UNORDERED(arr,i) (AT(arr,i)=PEEK(arr),(arr).data[--(arr).len])

#define ARR_ENSURE_CAP(arr) \
((size_t)(arr).len >= (size_t)(arr).cap ? ( \
    (arr).cap = (arr).cap ? (arr).cap * 2 : 8, \
    (arr).data = realloc((arr).data, (arr).cap * sizeof(*(arr).data)), \
    assert((arr).data != NULL && "went OOM"), \
    (arr) \
) : (arr))

#define ENSURE_CAP(arr) ARR_ENSURE_CAP(arr)

#define ARR_PUSH(arr,x) \
(ENSURE_CAP(arr), (arr).data[(arr).len] = (x), (arr).len++)

#endif // UTILS_H


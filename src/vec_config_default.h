#ifndef INCLUDED_VEC_CONFIG_DEFAULT_H
#define INCLUDED_VEC_CONFIG_DEFAULT_H
#include <stddef.h>

#define VEC_INIT_CAPACITY 8
#define VEC_GROW_CAPACITY(n) (((n) << 1) + (n >> 1))

// If a different size type is desired
#if !defined(VEC_SIZE_TYPE)
typedef size_t vec_size_t;
#else
typedef VEC_SIZE_TYPE vec_size_t;
#endif

// Define any signature decoration for vector APIs
#define VEC_API(name) name

//
// Structure alignment helpers
//
#if defined(__GNUC__) || defined(__clang__)
#define VEC_PRE_ALIGN
#define VEC_POST_ALIGN __attribute__ ((aligned (8)))
#elif defined(WIN32)
#define VEC_PRE_ALIGN __declspec(align(8))
  #define VEC_POST_ALIGN
#endif

//
// Memory allocator overrides
//
#if !defined(VEC_MALLOC) && !defined(VEC_MALLOC) && !defined(VEC_FREE)
// No operations defined, re-direct to system allocator
#define VEC_MALLOC malloc
#define VEC_FREE free
#define VEC_REALLOC realloc
#else
#if !defined(VEC_MALLOC) || !defined(VEC_REALLOC) || !defined(VEC_FREE)
    #error "Missing memory override VEC_MALLOC, VEC_REALLOC or VEC_FREE."
  #endif
#endif

#endif // INCLUDED_VEC_CONFIG_DEFAULT_H

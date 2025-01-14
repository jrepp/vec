/** 
 * Copyright (c) 2014 rxi (https://github.com/rxi/vec)
 *
 * v0.3.x modifications (c) 2022 Jacob Repp (https://github.com/jrepp/vec)
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See https://github.com/rxi/vec/LICENSE for details.
 */

#ifndef INCLUDED_VEC_H
#define INCLUDED_VEC_H

#if defined(__cplusplus)
extern "C" {
#endif

//
// Define VEC_CONFIG_H before including the vec.h header to override the default
// definitions of allocation, size, alignment and vector growth.
//
#if !defined(VEC_CONFIG_H)
#define VEC_CONFIG_H "vec_config_default.h"
#endif

#include VEC_CONFIG_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>


// See CMakeLists.txt and package.json
#define VEC_VERSION "0.3.0"


// General error, the operation failed
#define VEC_ERR -1

// The vector is out of memory but reallocation is not allowed
#define VEC_ERR_NO_REALLOC -2

// The allocator is unable to return additional memory
#define VEC_ERR_NO_MEMORY -3

// 0 is used as a general OK signal
#define VEC_OK 0

// When doing indexed operations, a general error is returned when the
// item being indexed is not found
#define VEC_NOT_FOUND ((vec_size_t)VEC_ERR)

//
// Options for vectors
//
#define VEC_OWNS_MEMORY     0x10
#define VEC_ALLOW_REALLOC   0x20
#define VEC_OOM             0x01

//
// Combinations of options for vector initialization
//
#define VEC_DYNAMIC (VEC_OWNS_MEMORY|VEC_ALLOW_REALLOC)
#define VEC_FIXED_REALLOC (VEC_ALLOW_REALLOC)
#define VEC_FIXED (0)


// Optionally add assert into array access, the statement remains unchanged but
// but will break before access the array out of bounds.
#if defined(VEC_USE_CHECKED_ACCESS)
#define VEC_CHECK(v, i) assert(i < (v)->length)
#else
#define VEC_CHECK(v, i) ((void)1)
#endif // VEC_USE_CHECKED_ACCESS

//
// Count the dimensions of a fixed array
//
#define vec_countof(arr) \
  (sizeof(arr) / sizeof(arr[0]))


// Given a vector unpack it into arguments for the vector helper functions
#define vec_unpack_(v) \
  (uint8_t**)&(v)->data, &(v)->options, &(v)->length, &(v)->capacity, sizeof(*(v)->data)


// Declare a new vector type
#define vec_define_fields(T) \
   T *data; size_t options, length, capacity;


// Initialize vector fields
#define vec_init(v) \
  (void) ((v)->data = NULL, (v)->options = VEC_DYNAMIC, (v)->length = 0, (v)->capacity = 0)


// Initialize with a fixed vector, no reallocation
#define vec_init_with_fixed(v, ptr, capacity_) \
  (void) ((v)->data = (ptr), (v)->options = VEC_FIXED, (v)->length = 0, (v)->capacity = (capacity_))


// Initialize with a fixed vector, reallocate above capacity
#define vec_init_with_realloc(v, ptr, capacity_) \
  (void) ((v)->data = (ptr), (v)->options = VEC_FIXED_REALLOC, (v)->length = 0, (v)->capacity = (capacity_))


// Free vectory memory
#define vec_deinit(v) \
  ( (((v)->options & VEC_OWNS_MEMORY) ? VEC_FREE((v)->data) : (void)0), vec_init(v) )


// Length of vector in elements
#define vec_length(v) ((v)->length)


// Capacity of vector in elements
#define vec_capacity(v) ((v)->capacity)


// True when the vector has observed an oom condition, useful for error checking
// after batch operations such as pusharr that don't have a natural return code.
#define vec_oom(v) (((v)->options & VEC_OOM) == VEC_OOM)


// Availability of vector in elements
#define vec_available(v) ((v)->capacity - (v)->length)

// Push an element, returns VEC_OK or VEC_ERR
#define vec_push(v, val)                  \
  ( vec_expand_(vec_unpack_(v))           \
     ? VEC_ERR                            \
     : (                                  \
        (v)->data[(v)->length++] = (val), \
        VEC_OK                            \
       )                                  \
  )


// Push an element, returns length of vector
#define vec_pop(v) \
  ((v)->length > 0 ? (--(v)->length) : 0)


// Splice the start and count of the vector, adjust length to specified count
#define vec_splice(v, start, count)\
  ( vec_splice_(vec_unpack_(v), start, count),\
    (v)->length -= (count) )


// Swap count elements from the end to the start index of the front of vector
#define vec_swapsplice(v, start, count)\
  (                                               \
   vec_swapsplice_(vec_unpack_(v), start, count), \
   (v)->length -= (count)                         \
  )


// Insert `val` at specified `idx`, adjust contents up
#define vec_insert(v, idx, val)      \
  ( vec_insert_(vec_unpack_(v), idx) \
    ? VEC_ERR                        \
    : (                              \
       (v)->data[idx] = (val),       \
       (v)->length++,                \
       VEC_OK                        \
      )                              \
    )


// `qsort()` the contents using the given `fn`
#define vec_sort(v, fn)\
  qsort((v)->data, (v)->length, sizeof(*(v)->data), fn)


// `bsearch()` the contents using `key` and `fn` result in `idx`
#define vec_bsearch(v, key, idx, fn)                                    \
  do {                                                                  \
    uint8_t* ptr = NULL;                                                \
    ptr = bsearch(key, (v)->data, (v)->length, sizeof(*(v)->data), fn); \
    *idx = ptr ?                                                        \
      (ptr - (uint8_t*)(v)->data) / sizeof(*((v)->data))                \
      : VEC_NOT_FOUND;                                                  \
  } while (0)


// Swap the elements at `idx1` and `idx2`
#define vec_swap(v, idx1, idx2)\
  vec_swap_(vec_unpack_(v), idx1, idx2)


// Truncate the vector to `len`
#define vec_truncate(v, len) \
  ((v)->length = (len) < (v)->length ? (len) : (v)->length)


// Truncate the vector to 0
#define vec_clear(v) \
  ((v)->length = 0)


// True when the vector is empty
#define vec_empty(v) \
  ((v)->length == 0)


// Get an element at index
#define vec_get(v, i) \
  ((v)->data[(VEC_CHECK(v, i), i)])


// Get an element by pointer at index
#define vec_get_ptr(v, i) \
  (&((v)->data[(VEC_CHECK(v, i), i)]))


// Return the first element (assumes length > 0)
#define vec_first(v) \
  ((v)->data[(VEC_CHECK(v, 0), 0)])


// Returns the last element (assumes length > 0)
#define vec_last(v) \
  ((v)->data[(VEC_CHECK(v, (v)->length - 1), (v)->length - 1)])


// Swap the data of src into dst, releasing dst before the swap
#define vec_swap_data(dst, src)        \
  do {                                 \
    if (((dst)->data) == ((src)->data))    \
      break;                           \
    vec_deinit(dst);                   \
    (dst)->data     = (src)->data;     \
    (dst)->options  = (src)->options;  \
    (dst)->length   = (src)->length;   \
    (dst)->capacity = (src)->capacity; \
    vec_init(src);                     \
  } while(0);


// Reserve space for `n` elements
#define vec_reserve(v, n)          \
  (vec_reserve_(vec_unpack_(v), n) \
    ? (VEC_ERR)                    \
    : (VEC_OK)                     \
  )


// Truncate the capacity of the vector to it's current length
#define vec_compact(v) \
  (vec_compact_(vec_unpack_(v)) \
    ? VEC_ERR : VEC_OK)


// Reserve and copy the values from a source array
#define vec_pusharr(v, arr, count)                                       \
  do {                                                                   \
    vec_size_t i__, n__ = (count);                                       \
    if (vec_reserve_(vec_unpack_(v), (v)->length + n__) != 0) break;     \
    for (i__ = 0; i__ < n__; i__++) {                                    \
      (v)->data[(v)->length++] = (arr)[i__];                             \
    }                                                                    \
  } while (0)


// Extend vector `v` with the elements from `v2`
#define vec_extend(v, v2)\
  vec_pusharr((v), (v2)->data, (v2)->length)


// Find `val` in vector, populates `idx` with result
#define vec_find(v, val, idx)\
  do {                                                \
    for ((idx) = 0; (idx) < (v)->length; (idx)++) {   \
      if ((v)->data[(idx)] == (val)) break;           \
    }                                                 \
    if ((idx) == (v)->length) (idx) = VEC_NOT_FOUND;  \
  } while (0)


// Find `val` in vector, populates `idx` with result
#define vec_rfind(v, val, idx)\
  do {                                                \
    for ((idx) = (v)->length - 1; (idx) < (v)->length; (idx)--) {   \
      if ((v)->data[(idx)] == (val)) break;           \
    }                                                 \
    if ((idx) == (v)->length) (idx) = VEC_NOT_FOUND;  \
  } while (0)


// Remove an element from the vector by value
#define vec_remove(v, val)                    \
  do {                                        \
    vec_size_t idx__;                             \
    vec_find(v, val, idx__);                  \
    if (idx__ != VEC_NOT_FOUND) {             \
      vec_splice(v, idx__, 1);                \
    }                                         \
  } while (0)


// Reverse the contents of a vector
#define vec_reverse(v)                             \
  do {                                             \
    vec_size_t i__ = (v)->length >> 1;                 \
    while (i__--) {                                \
      vec_swap((v), i__, (v)->length - (i__ + 1)); \
    }                                              \
  } while (0)


// Simply execute a function on each valid element of v
#define vec_each(v, f, ...) \
 for (vec_size_t i__ = 0, l__ = vec_length(v); i__ < l__; ++i__) { \
    (void)f((v)->data[i__] , ## __VA_ARGS__ );  \
  }


// Simply execute a function on each valid element of v by pointer
#define vec_each_ptr(v, f, ...) \
 for (vec_size_t i__ = 0, l__ = vec_length(v); i__ < l__; ++i__) { \
    (void)f(&((v)->data[i__]) , ## __VA_ARGS__ );     \
  }


// Iterate over each element of the vector, `var` is the element and `iter` is the index
#define vec_foreach(v, var, iter)                                  \
  if ((v)->length > 0)                                             \
    for ((iter) = 0;                                               \
         (iter) < (v)->length && (((var) = (v)->data[(iter)]), 1); \
         ++(iter))


// Iterate over each element of the vector in reverse, `var` is the element and `iter` is the index
#define vec_foreach_rev(v, var, iter)                     \
  if ((v)->length > 0)                                    \
    for ((iter) = (v)->length - 1;                        \
         (iter) >= 0 && (((var) = (v)->data[(iter)]), 1); \
         --(iter))


// Iterate over each element of the vector, `var` is a pointer to the element and `iter` is the index
#define vec_foreach_ptr(v, var, iter)                               \
  if ((v)->length > 0)                                              \
    for ((iter) = 0;                                                \
         (iter) < (v)->length && (((var) = &(v)->data[(iter)]), 1); \
         ++(iter))


// Iterate over each element of the vector in reverse, `var` is the pointer to the element and `iter` is the index
#define vec_foreach_ptr_rev(v, var, iter)                  \
  if ((v)->length > 0)                                     \
    for ((iter) = (v)->length - 1;                         \
         (iter) >= 0 && (((var) = &(v)->data[(iter)]), 1); \
         --(iter))



// Apply v2[i] = f(v[i], ...) for each element v
#define vec_map(dst, src, f, ...) \
  do {                    \
    if (VEC_OK != vec_reserve((dst), vec_length(src))) { \
      break;                   \
    } \
    (dst)->length = (src)->length; \
    VEC_TYPEOF((src)->data[0]) *s__ = &(src)->data[0], \
                               *e__ = &(src)->data[(src)->length], \
                               *d__ = &(dst)->data[0]; \
    for (; s__ < e__; ++s__, ++d__) { \
      *d__ = (f)(*s__ , ## __VA_ARGS__ );                        \
    } \
  } while(0);


// Apply dst[i] = f(src[i], ...) for each element v in reverse
#define vec_map_rev(dst, src, f, ...) \
  do {                             \
    if (VEC_OK != vec_reserve((dst), vec_length(src))) { \
      break;                                \
    }; \
    (dst)->length = (src)->length;    \
    VEC_TYPEOF((src)->data[0]) *s__ = &(src)->data[(src)->length - 1], \
                               *e__ = &(src)->data[-1],  \
                               *d__ = &(dst)->data[0]; \
    for (; s__ > e__; --s__, ++d__) { \
      *d__ = (f)(*s__, ##__VA_ARGS__); \
    } \
  } while (0);


// Apply dst[i] = f(&src[i], ...) for each element of v
#define vec_map_ptr(dst, src, f, ...) \
  do {                    \
    if (VEC_OK != vec_reserve((dst), vec_length(src))) { \
      break;                         \
    } \
    (dst)->length = (src)->length; \
    VEC_TYPEOF((src)->data[0]) *s__ = &(src)->data[0], \
                               *e__ = &(src)->data[(src)->length], \
                               *d__ = &(dst)->data[0]; \
    for (; s__ < e__; ++s__, ++d__) { \
      *d__ = (f)(s__, ## __VA_ARGS__ );                        \
    } \
  } while(0);


// Apply dst[i] = f(&src[i], ...) for each element of v in reverse
#define vec_map_ptr_rev(dst, src, f, ...) \
  do {                                 \
    if (VEC_OK != vec_reserve((dst), vec_length(src))) { \
      break;                               \
    } \
    (dst)->length = (src)->length; \
    VEC_TYPEOF((src)->data[0]) *s__ = &(src)->data[(src)->length - 1], \
                               *e__ = &(src)->data[-1],       \
                               *d__ = &(dst)->data[0];   \
    for (; s__ > e__; --s__, ++d__) { \
      *d__ = (f)(s__, ##__VA_ARGS__); \
    }                                    \
  } while (0);


// Apply ov = f(v[i], ov) for each element of v
#define vec_fold(v, ov, f, ...) \
  do {                     \
    VEC_TYPEOF((v)->data[0]) *s__ = &(v)->data[0], \
                             *e__ = &(v)->data[(v)->length]; \
    for (; s__ < e__; ++s__) { \
      ov = (f)(ov, *s__ , ## __VA_ARGS__ );                       \
    } \
  } while(0);


// Apply ov = f(&v[i], ov) for each element of v
#define vec_fold_ptr(v, ov, f, ...) \
  do {                     \
    VEC_TYPEOF((v)->data[0]) *s__ = &(v)->data[0], \
                             *e__ = &(v)->data[(v)->length]; \
    for (; s__ < e__; ++s__) { \
      ov = (f)(ov, s__ , ## __VA_ARGS__ );                       \
    } \
  } while(0);


// Apply (expr) (v[i]) for each element of v
#define vec_fold_expr(v, expr, ...) \
  do {                     \
    VEC_TYPEOF((v)->data[0]) *s__ = &(v)->data[0], \
                             *e__ = &(v)->data[(v)->length]; \
    for (; s__ < e__; ++s__) { \
      expr (*s__ , ## __VA_ARGS__ );                       \
    } \
  } while(0);


int VEC_API(vec_expand_)(uint8_t **data, vec_size_t *options, const size_t *length, vec_size_t *capacity, vec_size_t memsz);

int VEC_API(vec_reserve_)(uint8_t **data, vec_size_t *options, const size_t *length, vec_size_t *capacity, vec_size_t memsz, vec_size_t n);

int VEC_API(vec_compact_)(uint8_t **data, vec_size_t *options, const size_t *length, vec_size_t *capacity, vec_size_t memsz);

int VEC_API(vec_insert_)(uint8_t **data, vec_size_t *options, vec_size_t *length, vec_size_t *capacity, vec_size_t memsz, vec_size_t idx);

void VEC_API(vec_splice_)(uint8_t *const *data, const vec_size_t *options, const size_t *length, const vec_size_t *capacity, vec_size_t memsz, vec_size_t start, vec_size_t count);

void VEC_API(vec_swapsplice_)(uint8_t *const *data, const vec_size_t *options, const size_t *length, const vec_size_t *capacity, vec_size_t memsz, vec_size_t start, vec_size_t count);

void VEC_API(vec_swap_)(uint8_t *const *data, const vec_size_t *options, const size_t *length, const vec_size_t *capacity, vec_size_t memsz, vec_size_t idx1, vec_size_t idx2);

//
// Pre-defined stand-alone vector structure types
//
// All that is required to use the vector APIs is to have the vector fields in your type
// by passing a pointer to your type the fields will be expanded into the API
//
typedef VEC_PRE_ALIGN struct { vec_define_fields(void*) } vec_void_t VEC_POST_ALIGN;
typedef VEC_PRE_ALIGN struct { vec_define_fields(char*) } vec_str_t VEC_POST_ALIGN;
typedef VEC_PRE_ALIGN struct { vec_define_fields(int) } vec_int_t VEC_POST_ALIGN;
typedef VEC_PRE_ALIGN struct { vec_define_fields(int32_t) } vec_int32_t VEC_POST_ALIGN;
typedef VEC_PRE_ALIGN struct { vec_define_fields(uint32_t) } vec_uint32_t VEC_POST_ALIGN;
typedef VEC_PRE_ALIGN struct { vec_define_fields(int64_t) } vec_int64_t VEC_POST_ALIGN;
typedef VEC_PRE_ALIGN struct { vec_define_fields(uint64_t) } vec_uint64_t VEC_POST_ALIGN;
typedef VEC_PRE_ALIGN struct { vec_define_fields(char) } vec_char_t VEC_POST_ALIGN;
typedef VEC_PRE_ALIGN struct { vec_define_fields(uint8_t) } vec_uint8_t VEC_POST_ALIGN;
typedef VEC_PRE_ALIGN struct { vec_define_fields(float) } vec_float_t VEC_POST_ALIGN;
typedef VEC_PRE_ALIGN struct { vec_define_fields(double) } vec_double_t VEC_POST_ALIGN;


#if defined(__cplusplus)
}
#endif

#endif // INCLUDED_VEC_H

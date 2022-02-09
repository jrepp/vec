/** 
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#ifndef INCLUDED_VEC_H
#define INCLUDED_VEC_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define VEC_VERSION "0.3.0"
#define VEC_ERR -1
#define VEC_OK 0
#define VEC_NOT_FOUND ((size_t)VEC_ERR)

// If a different size type is desired
#if !defined(VEC_SIZE_TYPE)
typedef size_t vec_size_t;
#else
typedef VEC_SIZE_TYPE vec_size_t;
#endif

#define vec_unpack_(v) \
  (uint8_t**)&(v)->data, &(v)->length, &(v)->capacity, sizeof(*(v)->data)


#define vec_t(T) \
  struct { T *data; vec_size_t length, capacity; }


#define vec_init(v) \
  (void) ((v)->data = NULL, (v)->length = 0, (v)->capacity = 0)


#define vec_deinit(v) \
  ( VEC_FREE((v)->data), vec_init(v) )


#define vec_push(v, val)                  \
  ( vec_expand_(vec_unpack_(v))           \
     ? VEC_ERR                            \
     : (                                  \
        (v)->data[(v)->length++] = (val), \
        VEC_OK                            \
       )                                  \
  )


#define vec_pop(v) \
  (v)->data[--(v)->length]


#define vec_splice(v, start, count)\
  ( vec_splice_(vec_unpack_(v), start, count),\
    (v)->length -= (count) )


#define vec_swapsplice(v, start, count)\
  (                                               \
   vec_swapsplice_(vec_unpack_(v), start, count), \
   (v)->length -= (count)                         \
  )


#define vec_insert(v, idx, val)      \
  ( vec_insert_(vec_unpack_(v), idx) \
    ? VEC_ERR                        \
    : (                              \
       (v)->data[idx] = (val),       \
       (v)->length++,                \
       VEC_OK                        \
      )                              \
    )
    

#define vec_sort(v, fn)\
  qsort((v)->data, (v)->length, sizeof(*(v)->data), fn)


#define vec_bsearch(v, key, idx, fn)                                    \
  do {                                                                  \
    uint8_t* ptr = NULL;                                                \
    ptr = bsearch(key, (v)->data, (v)->length, sizeof(*(v)->data), fn); \
    *idx = ptr ?                                                        \
      (ptr - (uint8_t*)(v)->data) / sizeof(*((v)->data))                \
      : VEC_NOT_FOUND;                                                  \
  } while (0)


#define vec_swap(v, idx1, idx2)\
  vec_swap_(vec_unpack_(v), idx1, idx2)


#define vec_truncate(v, len) \
  ((v)->length = (len) < (v)->length ? (len) : (v)->length)


#define vec_clear(v) \
  ((v)->length = 0)


#define vec_first(v) \
  ((v)->data[0])


#define vec_last(v) \
  ((v)->data[(v)->length - 1])


#define vec_reserve(v, n)          \
  (vec_reserve_(vec_unpack_(v), n) \
    ? (VEC_ERR)                    \
    : (VEC_OK)                     \
  )


#define vec_compact(v) \
  (vec_compact_(vec_unpack_(v)) \
    ? VEC_ERR : VEC_OK)


#define vec_pusharr(v, arr, count)                                       \
  do {                                                                   \
    vec_size_t i__, n__ = (count);                                           \
    if (vec_reserve_po2_(vec_unpack_(v), (v)->length + n__) != 0) break; \
    for (i__ = 0; i__ < n__; i__++) {                                    \
      (v)->data[(v)->length++] = (arr)[i__];                             \
    }                                                                    \
  } while (0)


#define vec_extend(v, v2)\
  vec_pusharr((v), (v2)->data, (v2)->length)

#define vec_find(v, val, idx)\
  do {                                                \
    for ((idx) = 0; (idx) < (v)->length; (idx)++) {   \
      if ((v)->data[(idx)] == (val)) break;           \
    }                                                 \
    if ((idx) == (v)->length) (idx) = VEC_NOT_FOUND;  \
  } while (0)


#define vec_remove(v, val)                    \
  do {                                        \
    vec_size_t idx__;                             \
    vec_find(v, val, idx__);                  \
    if (idx__ != VEC_NOT_FOUND) {             \
      vec_splice(v, idx__, 1);                \
    }                                         \
  } while (0)


#define vec_reverse(v)                             \
  do {                                             \
    vec_size_t i__ = (v)->length >> 1;                 \
    while (i__--) {                                \
      vec_swap((v), i__, (v)->length - (i__ + 1)); \
    }                                              \
  } while (0)


#define vec_foreach(v, var, iter)                                  \
  if ((v)->length > 0)                                             \
    for ((iter) = 0;                                               \
         (iter) < (v)->length && (((var) = (v)->data[(iter)]), 1); \
         ++(iter))


#define vec_foreach_rev(v, var, iter)                     \
  if ((v)->length > 0)                                    \
    for ((iter) = (v)->length - 1;                        \
         (iter) >= 0 && (((var) = (v)->data[(iter)]), 1); \
         --(iter))


#define vec_foreach_ptr(v, var, iter)                               \
  if ((v)->length > 0)                                              \
    for ((iter) = 0;                                                \
         (iter) < (v)->length && (((var) = &(v)->data[(iter)]), 1); \
         ++(iter))


#define vec_foreach_ptr_rev(v, var, iter)                  \
  if ((v)->length > 0)                                     \
    for ((iter) = (v)->length - 1;                         \
         (iter) >= 0 && (((var) = &(v)->data[(iter)]), 1); \
         --(iter))



#if defined(VEC_FREE) && defined(VEC_REALLOC)
    // Both defined, no error
#elif !defined(VEC_REALLOC) && !defined(VEC_FREE)
    // Neither defined, use stdlib
    #define VEC_FREE free
    #define VEC_REALLOC realloc
#else
    #error "Must define all or none of VEC_FREE and VEC_REALLOC."
#endif


int vec_expand_(uint8_t **data, const vec_size_t *length, vec_size_t *capacity, vec_size_t memsz);

int vec_reserve_(uint8_t **data, const vec_size_t *length, vec_size_t *capacity, vec_size_t memsz, vec_size_t n);

int vec_reserve_po2_(uint8_t **data, vec_size_t *length, vec_size_t *capacity, vec_size_t memsz, vec_size_t n);

int vec_compact_(uint8_t **data, const vec_size_t *length, vec_size_t *capacity, vec_size_t memsz);

int vec_insert_(uint8_t **data, vec_size_t *length, vec_size_t *capacity, vec_size_t memsz, vec_size_t idx);

void vec_splice_(uint8_t *const *data, const vec_size_t *length, const vec_size_t *capacity, vec_size_t memsz, vec_size_t start, vec_size_t count);

void vec_swapsplice_(uint8_t *const *data, const vec_size_t *length, const vec_size_t *capacity, vec_size_t memsz, vec_size_t start, vec_size_t count);

void vec_swap_(uint8_t *const *data, const vec_size_t *length, const vec_size_t *capacity, vec_size_t memsz, vec_size_t idx1, vec_size_t idx2);


typedef vec_t(void*) vec_void_t;
typedef vec_t(char*) vec_str_t;
typedef vec_t(int) vec_int_t;
typedef vec_t(char) vec_char_t;
typedef vec_t(float) vec_float_t;
typedef vec_t(double) vec_double_t;

#endif // INCLUDED_VEC_H

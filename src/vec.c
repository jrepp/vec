/** 
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include "vec.h"
#include <string.h>

int vec_expand_(uint8_t **data, const vec_size_t *length, vec_size_t *capacity, vec_size_t memsz) {
  if (*length + 1 > *capacity) {
    void *ptr;
    vec_size_t n = (*capacity == 0) ? VEC_INIT_CAPACITY : VEC_GROW_CAPACITY(*capacity);
    ptr = VEC_REALLOC(*data, n * memsz);
    if (ptr == NULL) {
        return VEC_ERR_MEMORY;
    }
    *data = ptr;
    *capacity = n;
  }
  return VEC_OK;
}


int vec_reserve_(uint8_t **data, const vec_size_t *length, vec_size_t *capacity, vec_size_t memsz, vec_size_t n) {
  (void) length;
  if (n > *capacity) {
    uint8_t *ptr = VEC_REALLOC(*data, n * memsz);
    if (ptr == NULL) {
        return VEC_ERR_MEMORY;
    }
    *data = ptr;
    *capacity = n;
  }
  return VEC_OK;
}


int vec_reserve_po2_(uint8_t **data, vec_size_t *length, vec_size_t *capacity, vec_size_t memsz, vec_size_t n) {
  vec_size_t n2 = 1;
  if (n == 0) {
    return VEC_OK;
  }
  while (n2 < n) {
    n2 <<= 1;
  }
  return vec_reserve_(data, length, capacity, memsz, n2);
}


int vec_compact_(uint8_t **data, const vec_size_t *length, vec_size_t *capacity, vec_size_t memsz) {
  if (*length == 0) {
    VEC_FREE(*data);
    *data = NULL;
    *capacity = 0;
  } else {
    void *ptr;
    vec_size_t n = *length;
    ptr = VEC_REALLOC(*data, n * memsz);
    if (ptr == NULL) {
        return VEC_ERR_MEMORY;
    }
    *capacity = n;
    *data = ptr;
  }
  return VEC_OK;
}


int vec_insert_(uint8_t **data, vec_size_t *length, vec_size_t *capacity, vec_size_t memsz, vec_size_t idx) {
  int err = vec_expand_(data, length, capacity, memsz);
  if (err != VEC_OK) {
    return err;
  }
  memmove(*data + (idx + 1) * memsz,
          *data + idx * memsz,
          (*length - idx) * memsz);
  return VEC_OK;
}


void vec_splice_(uint8_t * const*data, const vec_size_t *length, const vec_size_t *capacity, vec_size_t memsz, vec_size_t start, vec_size_t count) {
  (void) capacity;
  memmove(*data + start * memsz,
          *data + (start + count) * memsz,
          (*length - start - count) * memsz);
}


void vec_swapsplice_(uint8_t *const *data, const vec_size_t *length, const vec_size_t *capacity, vec_size_t memsz, vec_size_t start, vec_size_t count) {
  (void) capacity;
  memmove(*data + start * memsz,
          *data + (*length - count) * memsz,
          count * memsz);
}


void vec_swap_(uint8_t *const *data, const vec_size_t *length, const vec_size_t *capacity, vec_size_t memsz, vec_size_t idx1, vec_size_t idx2) {
  (void) length;
  (void) capacity;
  uint8_t *a, *b, tmp;
  size_t count;

  if (idx1 == idx2) {
      return;
  }
  a = *data + idx1 * memsz;
  b = *data + idx2 * memsz;
  count = memsz;
  while (count--) {
    tmp = *a;
    *a = *b;
    *b = tmp;
    a++, b++;
  }
}


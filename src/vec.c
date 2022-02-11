/** 
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include "vec.h"
#include <string.h>

static uint8_t *vec_alloc_mem_(uint8_t *existing, vec_size_t *options, size_t existing_bytes, size_t new_bytes) {
  // If the vector doesn't own memory a new region must be acquired, do not release the old region
  if (0 == (*options & VEC_OWNS_MEMORY)) {
    uint8_t *new_region = VEC_MALLOC(new_bytes);
    if (new_region) {
      memcpy(new_region, existing, existing_bytes);
      *options |= VEC_OWNS_MEMORY;
      return new_region;
    }
    return NULL;
  }

  return VEC_REALLOC(existing, new_bytes);
}

int vec_expand_(uint8_t **data, vec_size_t *options, const vec_size_t *length, vec_size_t *capacity, vec_size_t memsz) {
  if (*length + 1 > *capacity) {
    if (0 == (*options & VEC_ALLOW_REALLOC)) {
      return VEC_ERR_NO_REALLOC;
    }
    size_t new_capacity = (*capacity == 0) ? VEC_INIT_CAPACITY : VEC_GROW_CAPACITY(*capacity);
    uint8_t* ptr = vec_alloc_mem_(*data, options, *length * memsz, new_capacity * memsz);
    if (ptr == NULL) {
        return VEC_ERR_NO_MEMORY;
    }
    *data = ptr;
    *capacity = new_capacity;
  }
  return VEC_OK;
}


int vec_reserve_(uint8_t **data, vec_size_t *options, const vec_size_t *length, vec_size_t *capacity, vec_size_t memsz, vec_size_t n) {
  (void) length;
  if (n > *capacity) {
    if (0 == (*options & VEC_ALLOW_REALLOC)) {
      return VEC_ERR_NO_REALLOC;
    }
    uint8_t *ptr = vec_alloc_mem_(*data, options, *length * memsz, n * memsz);
    if (ptr == NULL) {
        return VEC_ERR_NO_MEMORY;
    }
    *data = ptr;
    *capacity = n;
  }
  return VEC_OK;
}


int vec_compact_(uint8_t **data, vec_size_t *options, const size_t *length, vec_size_t *capacity, vec_size_t memsz) {
  if (*length == 0) {
    if (*options & VEC_OWNS_MEMORY) {
      VEC_FREE(*data);
    }
    *data = NULL;
    *capacity = 0;
  } else {
    if (0 == (*options & VEC_ALLOW_REALLOC)) {
      return VEC_ERR_NO_REALLOC;
    }

    vec_size_t n = *length;
    uint8_t *ptr = vec_alloc_mem_(*data, options, *capacity * memsz, n * memsz);
    if (ptr == NULL) {
        return VEC_ERR_NO_MEMORY;
    }
    *capacity = n;
    *data = ptr;
  }
  return VEC_OK;
}


int vec_insert_(uint8_t **data, vec_size_t *options, size_t *length, vec_size_t *capacity, vec_size_t memsz, vec_size_t idx) {
  int err = vec_expand_(data, options, length, capacity, memsz);
  if (err != VEC_OK) {
    return err;
  }
  memmove(*data + (idx + 1) * memsz,
          *data + idx * memsz,
          (*length - idx) * memsz);
  return VEC_OK;
}


void vec_splice_(uint8_t * const*data, const vec_size_t *options, const size_t *length, const vec_size_t *capacity, vec_size_t memsz, vec_size_t start, vec_size_t count) {
  (void) options;
  (void) capacity;
  memmove(*data + start * memsz,
          *data + (start + count) * memsz,
          (*length - start - count) * memsz);
}


void vec_swapsplice_(uint8_t *const *data, const vec_size_t *options, const size_t *length, const vec_size_t *capacity, vec_size_t memsz, vec_size_t start, vec_size_t count) {
  (void) options;
  (void) capacity;
  memmove(*data + start * memsz,
          *data + (*length - count) * memsz,
          count * memsz);
}


void vec_swap_(uint8_t *const *data, const vec_size_t *options, const vec_size_t *length, const vec_size_t *capacity, vec_size_t memsz, vec_size_t idx1, vec_size_t idx2) {
  (void) options;
  (void) length;
  (void) capacity;
  uint8_t *a, *b, tmp;
  vec_size_t count;

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


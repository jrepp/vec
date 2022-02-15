/**
 * Copyright (c) 2014 rxi (https://github.com/rxi/vec)
 *
 * v0.3.x modifications (c) 2022 Jacob Repp (https://github.com/jrepp/vec)
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See https://github.com/rxi/vec/LICENSE for details.
 */

#include "test_help.h"

int test_vec_fixed() {
  // Initialize with a fixed array, do not re-alloc on overflow
  { test_section("vec_init_with_fixed");
    int32_t arr[32];
    vec_int_t v;
    vec_init_with_fixed(&v, arr, vec_countof(arr));
    test_assert(v.data == &arr[0]);

    for (size_t i = 0; i < vec_countof(arr); i++) vec_push(&v, i * 2);

    test_assert(vec_countof(arr) == vec_length(&v));
    test_assert(VEC_ERR == vec_push(&v, 32 * 2));
    test_assert(vec_countof(arr) == vec_length(&v));
    test_assert(stats_->malloc_count == 0);
    test_assert(stats_->realloc_count == 0);
    test_assert(v.data[1] == 2);
    test_assert(v.data[31] == 31 * 2);
    test_assert(!vec_empty(&v));

    // Fixed arrays cannot be compacted (no reallocation)
    test_assert(VEC_ERR == vec_compact(&v));

    // Fixed arrays without realloc cannot be reserved
    test_assert(VEC_ERR == vec_reserve(&v, 1000));

    vec_deinit(&v);
    test_assert(stats_->free_count == 0);
    test_assert(vec_empty(&v));
    test_assert(v.data == NULL);
  }

  { test_section("vec_init_with_realloc");
    int32_t arr[32];
    vec_int_t v;
    vec_init_with_realloc(&v, arr, vec_countof(arr));

    for (size_t i = 0; i < vec_countof(arr); i++) vec_push(&v, i * 2);

    test_assert(vec_length(&v) == vec_countof(arr));
    test_assert(stats_->malloc_count == 0 && stats_->realloc_count == 0);
    for (size_t i = vec_countof(arr); i < 1000; ++i) vec_push(&v, i * 2);
    test_assert(stats_->realloc_count > 0);
    test_assert(vec_length(&v) == 1000);
    test_assert(v.data[1] == 2);
    test_assert(v.data[999] == 999 * 2);
    test_assert(vec_push(&v, 10) == 0);
    vec_deinit(&v);
    test_assert(stats_->memory == 0);
    test_assert(v.data == NULL);
    test_assert(vec_empty(&v));
  }
  return 0;
}

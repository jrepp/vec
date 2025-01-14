/**
 * Copyright (c) 2014 rxi (https://github.com/rxi/vec)
 *
 * v0.3.x modifications (c) 2022 Jacob Repp (https://github.com/jrepp/vec)
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See https://github.com/rxi/vec/LICENSE for details.
 */

#include "test_help.h"

int test_vec_mem_failures() {
  { test_section("vec_reserve_realloc_failure");
    vec_int_t v;
    vec_init(&v);
    set_fail_realloc(1);
    test_assert(VEC_ERR == vec_reserve(&v, 1000));
    test_assert(0 == vec_capacity(&v));
    vec_deinit(&v);
    set_fail_realloc(0);
  }
  { test_section("vec_reserve_fixed_malloc_failure");
    vec_int_t v;
    int arr[100];
    vec_init_with_realloc(&v, &arr[0], vec_countof(arr));
    set_fail_malloc(1);
    test_assert(VEC_ERR == vec_reserve(&v, 1000));
    test_assert(vec_oom(&v));
    test_assert(vec_countof(arr) == vec_capacity(&v));
    test_assert(VEC_OK == vec_push(&v, 1111));
    test_assert(v.data[0] == 1111);

    set_fail_malloc(0);
    test_assert(VEC_OK == vec_reserve(&v, 1000));
    test_assert(1000 == vec_capacity(&v));
    test_assert(v.data[0] == 1111);
    int i = 1;
    while(vec_available(&v)) {
      vec_push(&v, i * 2);
    }
    test_assert(!vec_available(&v));
    test_assert(1000 == vec_length(&v));
    vec_deinit(&v);
    test_assert(stats_->memory == 0);
  }
  { test_section("vec_push_expand_failure");
    vec_int_t v;
    test_stats_t stats = *stats_;
    vec_init(&v);

    // reallocate failure mode - only fill to initial allocation
    for (int i = 0; i < (int)VEC_INIT_CAPACITY; ++i) vec_push(&v, i * 2);
    test_assert(stats_->malloc_count == stats.malloc_count + 1);
    test_assert(stats_->realloc_count == stats.realloc_count);
    test_assert(vec_available(&v) == 0);
    test_assert(vec_capacity(&v) == VEC_INIT_CAPACITY);
    test_assert(vec_length(&v) == VEC_INIT_CAPACITY);

    set_fail_realloc(1);

    vec_size_t len = vec_length(&v);
    test_assert(VEC_ERR == vec_push(&v, 1111));
    test_assert(len == vec_length(&v));
    test_assert(v.data[v.length - 1] == (int)((v.length - 1) * 2));
    test_assert(stats_->realloc_count == stats.realloc_count);
    set_fail_realloc(0);

    test_assert(VEC_OK == vec_push(&v, 2222));
    test_assert(vec_length(&v) == len + 1);
    test_assert(v.data[v.length - 1] == 2222);
    vec_deinit(&v);
    test_assert(stats_->free_count == stats.free_count + 1);
    test_assert(stats_->realloc_count == stats.realloc_count + 1);
  }
  { test_section("vec_insert_expand_failure");
    vec_int_t v;
    test_stats_t stats = *stats_;
    vec_init(&v);

    // reallocate failure mode - only fill to initial capacity
    for (int i = 0; i < (int)VEC_INIT_CAPACITY; ++i) vec_insert(&v, 0, i * 2);
    test_assert(stats_->malloc_count == stats.malloc_count + 1);
    test_assert(stats_->realloc_count == stats.realloc_count);
    test_assert(vec_available(&v) == 0);
    test_assert(vec_capacity(&v) == VEC_INIT_CAPACITY);

    set_fail_realloc(1);

    vec_size_t len = vec_length(&v);
    test_assert(VEC_ERR == vec_insert(&v, 0, 1111));
    test_assert(len == vec_length(&v));
    test_assert(v.data[0] == (int)((v.length - 1) * 2));
    test_assert(stats_->realloc_count == stats.realloc_count);
    set_fail_realloc(0);

    test_assert(VEC_OK == vec_insert(&v, 0, 2222));
    test_assert(vec_length(&v) == len + 1);
    test_assert(v.data[0] == 2222);
    vec_deinit(&v);
    test_assert(stats_->free_count == stats.free_count + 1);
    test_assert(stats_->realloc_count == stats.realloc_count + 1);
  }
  { test_section("vec_push_malloc_failure");
    vec_int_t v;
    vec_init(&v);
    set_fail_malloc(1);
    test_assert(VEC_ERR == vec_push(&v, 1));
    test_assert(vec_empty(&v));
    set_fail_malloc(0);
    vec_deinit(&v);
  }
  { test_section("vec_insert_malloc_failure");
    vec_int_t v;
    vec_init(&v);
    set_fail_malloc(1);
    test_assert(VEC_ERR == vec_insert(&v, 0, 1));
    test_assert(vec_empty(&v));
    set_fail_malloc(0);
    vec_deinit(&v);
  }
  { test_section("vec_pusharr_failure");
    vec_int_t v;
    vec_init(&v);
    int values[] = {1, 2, 3, 4, 5, 6, 7, 8};
    set_fail_malloc(1);
    vec_pusharr(&v, values, vec_countof(values));
    test_assert(vec_oom(&v));
    v.options &= ~VEC_OOM;
    test_assert(!vec_oom(&v));
    set_fail_malloc(0);
    vec_pusharr(&v, values, vec_countof(values));
    test_assert(!vec_oom(&v));
    set_fail_realloc(1);
    vec_pusharr(&v, values, vec_countof(values));
    set_fail_realloc(0);
    test_assert(vec_oom(&v));
    vec_deinit(&v);
  }
  { test_section("vec_extend_failure");
    vec_int_t v;
    vec_init(&v);

    int values[] = {1, 2, 3, 4, 5, 6, 7, 8};
    vec_int_t v2;
    vec_init_with_fixed(&v2, values, vec_countof(values));
    v2.length = vec_countof(values);

    set_fail_malloc(1);
    vec_extend(&v, &v2);
    test_assert(vec_oom(&v));
    v.options &= ~VEC_OOM;
    test_assert(!vec_oom(&v));
    set_fail_malloc(0);
    vec_extend(&v, &v2);
    test_assert(!vec_oom(&v));
    set_fail_realloc(1);
    vec_extend(&v, &v2);
    set_fail_realloc(0);
    test_assert(vec_oom(&v));
    vec_deinit(&v);
  }

  { test_section("vec_compact_failure");
    test_assert(stats_->memory == 0);
    vec_int_t v;
    vec_init(&v);

    // fill the array, ensure capacity > length
    for (int i = 0; i < 5000; ++i) vec_push(&v, i * 2);
    test_assert(!vec_empty(&v));
    test_assert(vec_length(&v) == 5000);
    test_assert(v.data[4999] == 4999 * 2);
    test_assert(vec_available(&v) > 0);
    int * data = v.data;
    vec_size_t capacity = vec_capacity(&v);

    // fail compact, ensure data intact
    set_fail_realloc(1);
    test_assert(VEC_ERR == vec_compact(&v));
    test_assert(vec_capacity(&v) == capacity);
    test_assert(data == v.data);
    set_fail_realloc(0);

    test_assert(VEC_OK == vec_compact(&v));
    test_assert(vec_capacity(&v) == vec_length(&v));

    vec_deinit(&v);
    test_assert(stats_->memory == 0);
  }
  { test_section("vec_push_fixed_malloc_failure");
    int arr[32];
    vec_int_t v;
    test_stats_t stats = *stats_;
    vec_init_with_realloc(&v, &arr[0], vec_countof(arr));

    // reallocate failure mode - only fill to initial capacity
    for (int i = 0; i < (int)vec_countof(arr); ++i) vec_push(&v, i * 2);
    test_assert(stats_->malloc_count == stats.malloc_count);
    test_assert(stats_->realloc_count == stats.realloc_count);
    test_assert(vec_available(&v) == 0);
    test_assert(vec_length(&v) == vec_countof(arr));

    set_fail_malloc(1);

    vec_size_t len = vec_length(&v);
    test_assert(VEC_ERR == vec_push(&v, 1111));
    test_assert(len == vec_length(&v));
    test_assert(v.data[v.length - 1] == (int)((v.length - 1) * 2));
    test_assert(stats_->realloc_count == stats.realloc_count);
    set_fail_malloc(0);

    test_assert(VEC_OK == vec_push(&v, 2222));
    test_assert(vec_length(&v) == len + 1);
    test_assert(v.data[v.length - 1] == 2222);
    vec_deinit(&v);
    test_assert(stats_->free_count == stats.free_count + 1);
    test_assert(stats_->malloc_count == stats.malloc_count + 1);
  }
  return 0;
}

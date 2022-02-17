/**
 * Copyright (c) 2014 rxi (https://github.com/rxi/vec)
 *
 * v0.3.x modifications (c) 2022 Jacob Repp (https://github.com/jrepp/vec)
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See https://github.com/rxi/vec/LICENSE for details.
 */

#include "test_help.h"

typedef struct {
  int64_t action;
  int64_t sum;
} each_with_args_context_t;

static const vec_size_t count = 1000;

static int64_t each_action = 0;
static int64_t each_sum = 0;

static int64_t power(int64_t value) {
  return value * 10;
}

static int64_t sum(int64_t out, int64_t in) {
  return out + in;
}

static int64_t power_ptr(int64_t *value) {
  return *value * 10;
}

static int64_t sum_ptr(int64_t out, int64_t *in) {
  return out + *in;
}

static void each(int64_t i) {
  each_action++;
  each_sum += i;
}

static void each_with_args(int64_t i, each_with_args_context_t *context) {
  assert(context != NULL);

  each_action++;
  each_sum += i;

  context->action++;
  context->sum += i;
}

static void each_ptr(int64_t *i) {
  each_action++;
  each_sum += *i;
}

static void each_ptr_with_args(int64_t *i, each_with_args_context_t *context) {
  assert(context != NULL);

  each_action++;
  each_sum += *i;

  context->action++;
  context->sum += *i;
}

static void fill_vec(vec_int64_t *v) {
  for (int64_t i = 0; i < (int64_t)count; ++i) {
    if (VEC_OK != vec_push(v, i)) {
      break;
    }
  }
}

static int is_power(vec_int64_t *v) {
  for (int64_t i = 0; i < (int64_t)v->length; ++i) {
    if (v->data[i] != i * 10) {
      return 0;
    }
  }
  return 1;
}

static int is_ascending(vec_int64_t *v) {
  vec_size_t idx;
  int64_t var;
  vec_foreach(v, var, idx) {
    if (idx + 1 < vec_length(v) && var > v->data[idx + 1]) {
      return 0;
    }
  }
  return 1;
}

static int is_descending(vec_int64_t *v) {
  vec_size_t idx;
  int64_t var;
  vec_foreach(v, var, idx) {
    if (idx + 1 < vec_length(v) && var < v->data[idx + 1]) {
      return 0;
    }
  }
  return 1;
}

int test_vec_functional() {
  { test_section("vec_map");
    vec_int64_t v, v2;
    vec_init(&v);
    vec_init(&v2);
    fill_vec(&v);
    fill_vec(&v2);
    vec_map(&v2, &v, power);
    test_assert(v2.length == count);
    test_assert(is_ascending(&v2));
    test_assert(is_power(&v2));
    vec_deinit(&v);
    vec_deinit(&v2);
  }

  { test_section("vec_map_ptr");
    vec_int64_t v, v2;
    vec_init(&v);
    vec_init(&v2);
    fill_vec(&v);
    fill_vec(&v2);
    vec_map_ptr(&v2, &v, power_ptr);
    test_assert(v2.length == count);
    test_assert(is_ascending(&v2));
    test_assert(is_power(&v2));
    vec_deinit(&v);
    vec_deinit(&v2);
  }

  { test_section("vec_map_rev");
    vec_int64_t v, v2;
    vec_init(&v);
    vec_init(&v2);
    fill_vec(&v);
    fill_vec(&v2);
    vec_map_rev(&v2, &v, power);
    test_assert(v2.length == count);
    test_assert(is_descending(&v2));
    vec_deinit(&v);
    vec_deinit(&v2);
  }

  { test_section("vec_map_ptr_rev");
    vec_int64_t v, v2;
    vec_init(&v);
    vec_init(&v2);
    fill_vec(&v);
    fill_vec(&v2);
    vec_map_ptr_rev(&v2, &v, power_ptr);
    test_assert(v2.length == count);
    test_assert(is_descending(&v2));
    vec_deinit(&v);
    vec_deinit(&v2);
  }

  { test_section("vec_fold");
    int64_t values[32];
    vec_int64_t v;
    int64_t output = 0;
    vec_init_with_fixed(&v, values, vec_countof(values));
    fill_vec(&v);
    vec_fold(&v, output, sum);
    test_assert(output == 496);
    vec_deinit(&v);
  }

  { test_section("vec_fold_ptr");
    int64_t values[32];
    vec_int64_t v;
    int64_t output = 0;
    vec_init_with_fixed(&v, values, vec_countof(values));
    fill_vec(&v);
    vec_fold_ptr(&v, output, sum_ptr);
    test_assert(output == 496);
    vec_deinit(&v);
  }

  { test_section("vec_fold_expr");
    int64_t values[32];
    vec_int64_t v;
    int64_t output = 0;
    vec_init_with_fixed(&v, values, vec_countof(values));
    fill_vec(&v);
    vec_fold_expr(&v, output +=);
    test_assert(output == 496);
    vec_deinit(&v);
  }

  { test_section("vec_each");
    int64_t values[32];
    vec_int64_t v;
    vec_init_with_fixed(&v, values, vec_countof(values));
    fill_vec(&v);

    each_action = 0;
    each_sum = 0;
    vec_each(&v, each);
    test_assert(each_action == vec_countof(values));
    test_assert(each_sum == 496);

    each_action = 0;
    each_sum = 0;
    each_with_args_context_t context = { 0, 0 };
    vec_each(&v, each_with_args, &context);
    test_assert(each_action == vec_countof(values));
    test_assert(each_sum == 496);
    test_assert(context.action == each_action);
    test_assert(context.sum == each_sum);

    vec_deinit(&v);
  }

  { test_section("vec_each_ptr");
    int64_t values[32];
    vec_int64_t v;
    vec_init_with_fixed(&v, values, vec_countof(values));
    fill_vec(&v);

    each_action = 0;
    each_sum = 0;
    vec_each_ptr(&v, each_ptr);
    test_assert(each_action == vec_countof(values));
    test_assert(each_sum == 496);

    each_action = 0;
    each_sum = 0;
    each_with_args_context_t context = { 0, 0 };
    vec_each_ptr(&v, each_ptr_with_args, &context);
    test_assert(each_action == vec_countof(values));
    test_assert(each_sum == 496);
    test_assert(context.action == each_action);
    test_assert(context.sum == each_sum);

    vec_deinit(&v);
  }

  return 0;
}

#include "test_help.h"

static const vec_size_t count = 1000;

static int64_t power(int64_t value) {
  return value * 10;
}

static int64_t sum(int64_t out, int64_t in) {
  return out + in;
}

static void fill_vec(vec_int64_t *v) {
  for (int64_t i = 0; i < (int64_t)count; ++i) {
    if (VEC_OK != vec_push(v, i)) {
      break;
    }
  }
}

static void test_assert_power(vec_int64_t *v) {
  int valid = 1;
  for (int64_t i = 0; i < (int64_t)v->length; ++i) {
    if (v->data[i] != i * 10) {
      valid = 0;
    }
  }
  test_assert(valid && "power check valid");
}


int test_vec_functional() {
  { test_section("vec_map_inplace");
    vec_int64_t v;
    vec_init(&v);
    fill_vec(&v);
    vec_map_inplace(&v, power);
    test_assert(v.length == count);
    test_assert_power(&v);
    vec_deinit(&v);
  }

  { test_section("vec_map");
    vec_int64_t v, v2;
    vec_init(&v);
    vec_init(&v2);
    fill_vec(&v);
    fill_vec(&v2);
    vec_map(&v, &v2, power);
    test_assert(v.length == count);
    test_assert_power(&v2);
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

  return 0;
}

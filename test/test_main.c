/**
 * Copyright (c) 2014 rxi (https://github.com/rxi/vec)
 *
 * v0.3.x modifications (c) 2022 Jacob Repp (https://github.com/jrepp/vec)
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See https://github.com/rxi/vec/LICENSE for details.
 */

#include "test_help.h"

extern int test_vec_ops();
extern int test_vec_custom();
extern int test_vec_fixed();
extern int test_vec_functional();
extern int test_vec_mem_failures();

typedef int (*test_func)(void);

typedef struct {
  const char *name;
  test_func func;
} test_suite_t;

test_suite_t tests[] = {
  { "vec_ops", test_vec_ops },
  { "vec_custom", test_vec_custom },
  { "vec_fixed", test_vec_fixed },
  { "vec_functional", test_vec_functional },
  { "vec_mem_failures", test_vec_mem_failures },
};

int main() {
  test_stats_t stats, total_stats;
  test_stats_init(&stats);
  test_stats_init(&total_stats);
  stats_ = &stats;
  int result = 0;
  for (size_t i = 0; i < vec_countof(tests); ++i) {
    printf("------------------------------------------------------------\n");
    printf("-- Tests: %s                                              --\n", tests[i].name);
    printf("------------------------------------------------------------\n");
    test_stats_init(stats_);
    if (0 != tests[i].func()) {
      result = -2;
    }
    test_stats_report(stats_);
    test_print_res();
    test_stats_copy_forward(stats_, &total_stats);
  }

  printf("============================================================\n");
  printf("Totals:\n");
  test_stats_report(&total_stats);
  stats_ = &total_stats;
  test_print_res();
  if (stats.fail_count > 0) {
    return -1;
  }

  return result;
}

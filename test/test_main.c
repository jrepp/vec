#include "test_help.h"

extern int test_vec_ops();
extern int test_vec_custom();
extern int test_vec_fixed();
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
  { "vec_mem_failures", test_vec_mem_failures }
};

int main() {
  test_stats_t stats;
  test_stats_init(&stats);
  stats_ = &stats;
  int result = 0;
  for (size_t i = 0; i < vec_countof(tests); ++i) {
    printf("------------------------------------------------------------\n");
    printf("-- Tests: %s                                              --\n", tests[i].name);
    printf("------------------------------------------------------------\n");
    test_stats_init(stats_);
    if (0 != tests[i].func()) {
      result = -1;
    }
    test_stats_report(&stats);
    test_print_res();
  }

  return result;
}

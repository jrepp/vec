
#include "test_help.h"

test_stats_t *stats_;

void test_stats_init(test_stats_t *stats) {
  memset(stats, 0, sizeof(test_stats_t));
}

void test_stats_report(const test_stats_t *stats) {
  printf("  inuse-memory: %zu bytes\n"
         "  high-memory:  %zu bytes\n"
         "  mallocs:      %zu\n"
         "  reallocs:     %zu\n"
         "  frees:        %zu\n",
         stats->memory,
         stats->high_memory,
         stats->malloc_count,
         stats->realloc_count,
         stats->free_count);
}

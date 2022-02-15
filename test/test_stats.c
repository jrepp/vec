/**
 * Copyright (c) 2014 rxi (https://github.com/rxi/vec)
 *
 * v0.3.x modifications (c) 2022 Jacob Repp (https://github.com/jrepp/vec)
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See https://github.com/rxi/vec/LICENSE for details.
 */

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

void test_stats_copy_forward(const test_stats_t *src, test_stats_t *dst) {
  dst->memory += src->memory;
  if (src->high_memory > dst->high_memory) {
    dst->high_memory = src->high_memory;
  }
  dst->malloc_count     += src->malloc_count;
  dst->realloc_count    += src->realloc_count;
  dst->free_count       += src->free_count;
  dst->pass_count       += src->pass_count;
  dst->fail_count       += src->fail_count;
}


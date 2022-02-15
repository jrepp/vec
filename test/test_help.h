/**
 * Copyright (c) 2014 rxi (https://github.com/rxi/vec)
 *
 * v0.3.x modifications (c) 2022 Jacob Repp (https://github.com/jrepp/vec)
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See https://github.com/rxi/vec/LICENSE for details.
 */

#ifndef INCLUDED_VEC_TEST_HELP_H
#define INCLUDED_VEC_TEST_HELP_H

// Override the allocators with test allocators
#define VEC_CONFIG_H "vec_config_test.h"
#include "vec.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>

// test stat structure for tracing statistical values across test runs
typedef struct {
  int pass_count;
  int fail_count;
  size_t malloc_count;
  size_t free_count;
  size_t realloc_count;
  size_t memory;
  size_t high_memory;
} test_stats_t;

// reference to current stats
extern test_stats_t *stats_;
void test_stats_init(test_stats_t *stats);
void test_stats_report(const test_stats_t *stats);
void test_stats_copy_forward(const test_stats_t *src, test_stats_t *dst);

// controls for the test memory allocator (see test_mem.c)
void set_fail_malloc(int enable);
void set_fail_realloc(int enable);

#define test_section(desc)\
  do {\
    printf("--- %s\n", desc);\
  } while (0)

#define test_assert(cond)\
  do {\
    int pass__ = cond;\
    printf("[%s] %s:%d: ", pass__ ? "PASS" : "FAIL", __FILE__, __LINE__);\
    printf((strlen(#cond) > 50 ? "%.47s...\n" : "%s\n"), #cond);\
    if (pass__) { stats_->pass_count++; } else { stats_->fail_count++; }\
  } while (0)

#define test_print_res()\
  do {\
    printf("------------------------------------------------------------\n");\
    printf("-- Results:   %3d Total    %3d Passed    %3d Failed       --\n",\
           stats_->pass_count + stats_->fail_count, stats_->pass_count, stats_->fail_count);\
    printf("------------------------------------------------------------\n");\
  } while (0)

#endif // INCLUDED_VEC_TEST_HELP_H

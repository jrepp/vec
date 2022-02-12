#ifndef VEC_TEST_VEC_CONFIG_H
#define VEC_TEST_VEC_CONFIG_H

#include <stddef.h>

#define VEC_MALLOC test_malloc
#define VEC_REALLOC test_realloc
#define VEC_FREE test_free

void *test_malloc(size_t bytes);
void *test_realloc(void *p, size_t bytes);
void test_free(void *p);

#include "vec_config_default.h"

#endif //VEC_TEST_VEC_CONFIG_H

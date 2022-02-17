#include "test_help.h"

#define ITEM_NAME_SIZE 32

typedef struct item_t {
    int a;
    int b;
    char name[ITEM_NAME_SIZE];
} item_t;

// Define a custom structure with additional fields
typedef struct {
    vec_define_fields(item_t)
    uint32_t other_field;
} vec_item_t;


void test_assert_item(item_t *i, int a, int b, const char *s) {
  test_assert(i->a == a);
  test_assert(i->b == b);
  test_assert(!strcmp(i->name, s));
}

int test_vec_custom() {
  { test_section("vec_custom_item");
    vec_item_t v;
    v.other_field = 42;
    vec_init(&v);
    item_t i = { 1, 2, "1" };
    vec_push(&v, i);
    i.a = 3, i.b = 4;
    vec_push(&v, i);
    test_assert_item(&vec_last(&v), 3, 4, "1");
    test_assert_item(&vec_get(&v, 0), 1, 2, "1");
    test_assert_item(&vec_first(&v), 1, 2, "1");
    test_assert(&vec_get(&v, 1) == vec_get_ptr(&v, 1));
    vec_pop(&v);
    vec_deinit(&v);

    // custom fields should always be preserved
    test_assert(v.other_field == 42);
  }
  return 0;
}

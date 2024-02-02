# :signal_strength: vec
A type-safe dynamic array implementation for C. 

Forked from the archived project: https://github.com/rxi/vec

* Merged some downstream contributions
* Cleaned up the docs
* Added tests and polish

## Features

* MIT License
* Modular - configure allocation size, functions, and behavior
* Small
* Type-safe
* Efficient
* Functional APIs: map, fold, each

![build workflow](https://github.com/jrepp/vec/actions/workflows/cmake.yml/badge.svg)
[![codecov](https://codecov.io/gh/jrepp/vec/branch/master/graph/badge.svg?token=HIFGP99YWK)](https://codecov.io/gh/jrepp/vec)

## Installation 
The [vec.c](src/vec.c?raw=1) and [vec.h](src/vec.h?raw=1) files can be dropped
into an existing C project and compiled along with it. If no overrides are required,
[vec_config_default.h](src/vec_config_default.h?raw=1) should also be supplied.


## Usage
Before using a vector it should first be initialized using the `vec_init()`
function. To release vector memory ensure that `vec_deinit()` is always called.
```c
vec_int_t v;
vec_init(&v);
vec_push(&v, 123);
vec_push(&v, 456);
vec_deinit(&v);
```

To access the elements of the vector directly the vector's `data` field can be
used.
```c
if (vec_length(&v) >= 2)
  printf("%d\n", v.data[1]); /* Prints the value at index 1 */
```

The vector memory can be queried using helper macros.
```c
printf("%zu\n", vec_length(&v)); /* Prints the length of the vector */
printf("%zu\n", vec_capacity(&v)); /* Prints the capacity of the vector */
printf("%zu\n", vec_available(&v)); /* Prints the available elements of the vector */
```

If the vector contains dynamic memory it's simple to release all contained values.
```c
vec_str_t strs;
vec_init(&strs);
vec_push(&strs, stdrup("Hello"));
vec_push(&strs, stdrup("World"));
vec_each(&strs, free);
vec_deinit(&strs);
```

## Configuring
Configuration is available through the VEC_CONFIG_H compile option. There are overrides for

* Array allocation strategy:`VEC_INIT_CAPACITY`, `VEC_GROW_CAPACITY`
* Memory allocation: `VEC_MALLOC`, `VEC_FREE`, `VEC_REALLOC`
* Array sizes types: `vec_size_t`
* Structure alignment: `VEC_PRE_ALIGN`, `VEC_POST_ALIGN`
* API function call semantics: `VEC_API`


## Types
vec.h provides the following predefined vector types:

| Contained Type   | Type name       |
|------------------|-----------------|
| `void*`          | `vec_void_t`    |
| `char*`          | `vec_str_t`     |
| `int_t`          | `vec_int_t`     |
| `int32_t`        | `vec_int32_t`   |
| `uint32_t`       | `vec_uint32_t`  |
| `int64_t`        | `vec_int64_t`   |
| `uint64_t`       | `vec_uint64_t`  |
| `char`           | `vec_char_t`    |
| `uint8_t`        | `vec_uint8_t`   |
| `float`          | `vec_float_t`   |
| `double`         | `vec_double_t`  |

To define a new vector type the `vec_define_fields()` macro should be used:
```c
/* Creates the type uint_vec_t for storing unsigned ints */
typedef struct { vec_define_fields(unsigned int) } uint_vec_t;

/* Add vector functionality to a model chunk type */
typedef struct { float x, y, z } point3f_t;
struct vec_chunk {
  struct vec_chunk *next;
  chunk_header_t header;
  vec_define_fields(point3f_t);
} vec_chunk_t;
```


# API
To preserve the type expression across calls, vector functions are macros. The parameter 
`v` in each function must be a *pointer* to a structure that contains the vector fields. 
Most APIs have a pointer version and may also have a reverse iteration option. The 
arguments of each API are expanded in a macro, to avoid side effects in your arguments.

Avoid side effects. The following actually increments `i` twice.
```c
vec_insert(&v, i++)
```


## `vec_define_fields(T)`

Embeds the required vec structure fields for values of type `T`.
```c
typedef VEC_PRE_ALIGN struct { vec_define_fields(double) } vec_double_t VEC_POST_ALIGN;
```

Fields can be embedded in any struct - additional fields can be defined
and will not be affected by the vec API. See [test_vec_custom.c](test/test_vec_custom.c?raw=1) for an example.


## `vec_init(v)`
Initialize the fields of a vector, this must be called before the vector can be used. This function
only clears the fields of the vector and does not allocate memory. If the fields of the 
vector are contained in another structure simply use `memset(ptr, 0, sizeof(mystruct))`
or `mystruct *p = calloc(1, sizeof(mystruct))` are sufficient.


## `vec_init_with_fixed(v, ptr, capacity)`
Initialize the fields of a vector with a pre-existing fixed-type array. This allocation will not be
grown by default.
```c
int arr[32];
vec_int_t v;
vec_init_with_fixed(&v, arr, vec_countof(arr));
assert(vec_capacity(&v) == vec_countof(arr))
```


## `vec_init_with_realloc(v, ptr, capacity)`
Initializes the fields of a vector with a pre-existing fixed type array, allowing reallocation. When the
length of the array exceeds the provided capacity the data will be reallocated preserving existing items.

When capacity is exceeded the vector will allocate a new larger buffer.
```c
int arr[32];
vec_int_t v;
vec_init_with_realloc(&v, arr, vec_countof(arr));
for (int i = 0; i < 34; ++i) { vec_push(&v, i); }
assert(vec_length(&v) == 33);
```


## `vec_deinit(v)`
De-initializes the vector, freeing the memory of the vector allocated during use.


## `vec_push(v, val)`
Pushes a value to the end of the vector. Returns `VEC_OK` if the operation was
successful, otherwise `VEC_ERR` is returned and the vector remains unchanged.


## `vec_pop(v)`
Removes the last element and returns the length of the vector. Will return
0 on an empty array.


## `vec_splice(v, start, count)`
Removes the number of values specified by `count`, starting at the index
`start`.
```c
vec_splice(&v, 2, 4); /* Removes the values at indices 2, 3, 4, and 5 */
```


## `vec_swapsplice(v, start, count)`
Removes the number of values specified by `count`, starting at the index
`start`; the removed values are replaced with the last `count` values of the
vector. This does not preserve ordering but is O(1).


## `vec_insert(v, idx, val)`
Inserts the value `val` at index `idx` shifting the elements after the index
to make room for the new value.
```c
/* Inserts the value 123 at the beginning of the vec */
vec_insert(&v, 0, 123);
```
Returns 0 if the operation was successful, otherwise -1 is returned and the
vector remains unchanged.


## `vec_sort(v, fn)`
Sorts the values of the vector; `fn` should be a qsort-compatible compare
function.


## `vec_bsearch(v, key, idx, fn)`
Performs a binary search for `key` on the vector; the elements must be in sorted 
order according to the qsort-compatible function `fn`. The value pointed to by
`idx` is filled with an index of `key` if found, -1 if not found.
```c
/* Searches the vector for the key 4*/
int key = 4;
int idx = -1;
vec_bsearch(&v, &key, &idx, compareIntegers);
/* idx != -1 if key was found... */
```


## `vec_swap(v, idx1, idx2)`
Swaps the values at the indices `idx1` and `idx2` with one another.


## `vec_clear(v)` / `vec_truncate(v, len)`
Truncates the vector's length to `0` or `len`. In the case of `vec_truncate`, if
`len` is greater than the vector's current length then no change is made.


## `vec_first(v)` / `vec_last(v)`
Returns the first or last value in the vector. This should not be used on an empty
vector as it performs a direct index into the data.


## `vec_reserve(v, n)`
Reserves capacity for at least `n` elements in the given vector;  if `n` is
less than the current capacity then `vec_reserve()` does nothing. Returns 0 if
the operation was successful, otherwise, -1 is returned and the vector remains
unchanged.


## `vec_compact(v)`
Reduces the vector's capacity to the smallest size required to store its
current number of values. Returns 0 if the operation is successful, otherwise
-1 is returned and the vector remains unchanged.


## `vec_pusharr(v, arr, count)` / `vec_extend(dst, src)`
Extend `v` by multiple source elements.

Pushes the contents of the array `arr` or the vector `v2` to the end of the vector `v`. 

These operations will grow the underlying array. If the reallocation fails the array
will be partially filled and `vec_oom` will return `1`.


## `vec_oom(v)`
Returns true when the underlying vector has experienced an out-of-memory condition.


## `vec_swap_data(dst, src)`
Swap the data from `src` into `dst`. Any data that exists in `dst` will be de-initialized and
freed first. After the swap `src` will be initialized to an empty state.


## `vec_find(v, val, idx)` / `vec_rfind(v, val, inx)`
Finds the first or last occurrence of the value `val` in the vector. 

* `idx` should be an int where the value's index will be written; 
* `idx` is set to VEC_NOT_FOUND if `val` cannot be found in the vector.


## `vec_remove(v, val)`
Removes the first occurrence of the value `val` from the vector. If the `val`
is not contained in the vector then `vec_remove()` does nothing.


## `vec_reverse(v)`
Reverses the order of the vector's values in place. For example, a vector
containing `4, 5, 6` would contain `6, 5, 4` after reversing.


## `vec_foreach[_ptr][_rev](v, var, iter)`
For-each macro expand to the initial portion of a for loop allowing in-place
ordered iteration forwards or reverse with the value and iterator (index) available
locally for a loop body.

* `var` should be a variable of the vector's contained type which stores the value on each iteration. 
* `iter` should be `vec_size_t` used to store the index during iteration.
* `_rev` versions of foreach iterate in reverse from `length-1` to `0`
* `_ptr` versions of foreach take the pointer of the element into `var`

```c
/* Iterates and prints the value and index of each value in the float vec */
vec_size_t i; float val;
vec_foreach(&v, val, i) {
  printf("%zu : %f\n", i, val);
}
```
```c
/* Iterates and prints the value and index of each value in the float vector */
vec_size_t i; float *val;
vec_foreach_ptr(&v, val, i) {
  printf("%zu : %f\n", i, *val);
}
```

## `vec_each[_ptr](v, f, ...)`
`vec_each` iteration is a simplified functional form of foreach. This form is ideal when you 
just need to call a function on each element by value or pointer. Additional parameters
can be passed to the function after the value by including them in the macro call.
```c
/* For vectors of pointers it is common to need to free each element before vec_deinit */
vec_each(&v, free);
```
```c
/* It's easy to also pass along an argument to a functional iterator */
void my_system(component_t *c, arg_t *a) {}
typedef struct { vec_define_fields(component_t) } vec_component_t;
vec_component_t v;
// ...
arg_t arg = { ... };
vec_each(&v, my_system, &arg);
```
* `_ptr` takes the address of the value position as an argument to `f`

## `vec_map[_ptr][_rev](dst, src, f, ...)`
`vec_map` allows mapping from one vector to another using a function and optional arguments. 

Example:
```c
int convert(int x, int c) { /* transform x */ }
vec_int_t v, v2;
/* initialize and push values into v */
vec_map(&v2, &v, convert, 100); /* converts each element in v -> v2 using the argument 100*/
```
* `_ptr` passes the value to the function by pointer
* `_rev` reverses the iteration over `src`
* arguments after `f` are passed along after `v`


## `vec_fold[_ptr](v, ov, f, ...)`
`vec_fold` provides a type-safe combination of the output value `ov` over the vector `v`

* `_ptr` passes the value to the fold function by pointer
* arguments after `f` are passed along after `v`
 

## `vec_fold_expr(v, expr, ...)`
`vec_fold_expr` generalizes the fold as an expression over the elements of the vector
anything that is a valid expr over the vector data may be used. The expression will
receive each value and the arguments after `expr`.


## License
This library is free software; you can redistribute it and/or modify it under
the terms of the MIT license. See [LICENSE](LICENSE) for details.


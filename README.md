# vec
A type-safe dynamic array implementation for C. 


## Features

* MIT License
* Modular - configure allocation size, functions and behaviour
* Small
* Type-safe
* Effecient

![build workflow](https://github.com/jrepp/vec/actions/workflows/cmake.yml/badge.svg)
[![codecov](https://codecov.io/gh/jrepp/vec/branch/master/graph/badge.svg?token=HIFGP99YWK)](https://codecov.io/gh/jrepp/vec)

## Installation 
The [vec.c](src/vec.c?raw=1) and [vec.h](src/vec.h?raw=1) files can be dropped
into an existing C project and compiled along with it. If no overrides are required,
[vec_config_default.h](src/vec_config_default.h?raw=1) should also be supplied.


## Usage
Before using a vector it should first be initialised using the `vec_init()`
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

## Configuring
Configuration is available through the VEC_CONFIG_H compile option. There are overrides for

* Array allocation strategy:`VEC_INIT_CAPACITY`, `VEC_GROW_CAPACITY`
* Memory allocation: `VEC_MALLOC`, `VEC_FREE`, `VEC_REALLOC`
* Array sizes types: `vec_size_t`
* Structure alignment: `VEC_PRE_ALIGN`, `VEC_POST_ALIGN`
* API function call semantics: `VEC_API`


## Types
vec.h provides the following predefined vector types:

| Contained Type | Type name    |
|----------------|--------------|
| void*          | vec_void_t   |
| char*          | vec_str_t    |
| int_t          | vec_int_t    |
| int32_t        | vec_int32_t  |
| uint32_t       | vec_uint32_t |
| int64_t        | vec_int64_t  |
| uint64_t       | vec_uint64_t |
| char           | vec_char_t   |
| uint8_t        | vec_uint8_t  |
| float          | vec_float_t  |
| double         | vec_double_t |

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


## Functions
All vector functions are macro functions. The parameter `v` in each function
should be a pointer to the vec struct which the operation is to be performed
on.

### vec\_define\_fields(T)
Embeds the required vec structure fields for values of type `T`.
```c
typedef VEC_PRE_ALIGN struct { vec_define_fields(double) } vec_double_t VEC_POST_ALIGN;
```

Fields can be embedded in any struct - additional fields can be defined
and will not be affected by the vec API. See [test_vec_custom.c](test/test_vec_custom.c?raw=1) for an example.

### vec\_init(v)
Initialises the vector, this must be called before the vector can be used. 

### vec\_init_with_fixed(v, ptr, capacity)
Initialises with a pre-existing fixed allocation

Fixed vectors Will not reallocate if the capacity is exhausted.
```c
int arr[32];
vec_int_t v;
vec_init_with_fixed(&v, &arr[0], vec_countof(arr));
```

### vec\_init_with_realloc(v, ptr, capacity)
Initialises with a pre-existing fixed allocation, allowing reallocation.

When capacity is exceeded the vector will allocate a new larger buffer.
```c
int arr[32];
vec_int_t v;
vec_init_with_realloc(&v, &arr[0], vec_countof(arr));
```

### vec\_deinit(v)
Deinitialises the vector, freeing the memory the vector allocated during use;
this should be called when we're finished with a vector.

### vec\_push(v, val)
Pushes a value to the end of the vector. Returns 0 if the operation was
successful, otherwise -1 is returned and the vector remains unchanged.

### vec\_pop(v)
Removes the last element and returns the length of the vector. Will return
0 on empty array.

### vec\_splice(v, start, count)
Removes the number of values specified by `count`, starting at the index
`start`.
```c
vec_splice(&v, 2, 4); /* Removes the values at indices 2, 3, 4 and 5 */
```

### vec\_swapsplice(v, start, count)
Removes the number of values specified by `count`, starting at the index
`start`; the removed values are replaced with the last `count` values of the
vector. This does not preserve ordering but is O(1).

### vec\_insert(v, idx, val)
Inserts the value `val` at index `idx` shifting the elements after the index
to make room for the new value.
```c
/* Inserts the value 123 at the beginning of the vec */
vec_insert(&v, 0, 123);
```
Returns 0 if the operation was successful, otherwise -1 is returned and the
vector remains unchanged.

### vec\_sort(v, fn)
Sorts the values of the vector; `fn` should be a qsort-compatible compare
function.

### vec\_bsearch(v, key, idx, fn)
Performs a binary search for `key` on the vector; the elements must be in sorted 
order according to the qsort-compatible function `fn`. The value pointed to by
`idx` is filled with index of `key` if found, -1 if not found.
```c
/* Searches the vector for the key 4*/
int key = 4;
int idx = -1;
vec_bsearch(&v, &key, &idx, compareIntegers);
/* idx != -1 if key was found... */
```

### vec\_swap(v, idx1, idx2)
Swaps the values at the indices `idx1` and `idx2` with one another.

### vec\_truncate(v, len)
Truncates the vector's length to `len`. If `len` is greater than the vector's
current length then no change is made.

### vec\_clear(v)
Clears all values from the vector reducing the vector's length to 0.

### vec\_first(v)
Returns the first value in the vector. This should not be used on an empty
vector.

### vec\_last(v)
Returns the last value in the vector. This should not be used on an empty
vector.

### vec\_reserve(v, n)
Reserves capacity for at least `n` elements in the given vector;  if `n` is
less than the current capacity then `vec_reserve()` does nothing. Returns 0 if
the operation was successful, otherwise -1 is returned and the vector remains
unchanged.

### vec\_compact(v)
Reduces the vector's capacity to the smallest size required to store its
current number of values. Returns 0 if the operation is successful, otherwise
-1 is returned and the vector remains unchanged.

### vec\_pusharr(v, arr, count)
Pushes the contents of the array `arr` to the end of the vector. `count` should
be the number of elements in the array.

### vec\_extend(v, v2)
Appends the contents of the `v2` vector to the `v` vector.

### vec\_find(v, val, idx)
Finds the first occurrence of the value `val` in the vector. `idx` should be an
int where the value's index will be written; `idx` is set to -1 if `val` could
not be found in the vector.

### vec\_rfind(v, val, idx)
Finds the last occurrence of the value `val` in the vector. `idx` should be an
int where the value's index will be written; `idx` is set to -1 if `val` could
not be found in the vector.

### vec\_remove(v, val)
Removes the first occurrence of the value `val` from the vector. If the `val`
is not contained in the vector then `vec_remove()` does nothing.

### vec\_reverse(v)
Reverses the order of the vector's values in place. For example, a vector
containing `4, 5, 6` would contain `6, 5, 4` after reversing.

### vec\_foreach(v, var, iter)
Iterates the values of the vector from the first to the last. `var` should be a
variable of the vector's contained type where the value will be stored with
each iteration. `iter` should be an int used to store the index during
iteration.
```c
/* Iterates and prints the value and index of each value in the float vec */
int i; float val;
vec_foreach(&v, val, i) {
  printf("%d : %f\n", i, val);
}
```

### vec\_foreach\_rev(v, var, iter)
Iterates the values of the vector from the last to the first. See
`vec_foreach()`

### vec\_foreach\_ptr(v, var, iter)
Iterates the value pointers of the vector from first to last. `var` should be a
variable of the vector's contained type's pointer. See `vec_foreach()`.
```c
/* Iterates and prints the value and index of each value in the float vector */
int i; float *val;
vec_foreach_ptr(&v, val, i) {
  printf("%d : %f\n", i, *val);
}
```

### vec\_foreach\_ptr\_rev(v, var, iter)
Iterates the value pointers of the vector from last to first. See
`vec_foreach_ptr()`


## License
This library is free software; you can redistribute it and/or modify it under
the terms of the MIT license. See [LICENSE](LICENSE) for details.


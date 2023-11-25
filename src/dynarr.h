#include "luna.h"

// shitty polymorphic dynamic array lib

// im not proud of this
// jk im very proud of it

#define dynarr(type) dynarr_##type

#define dynarr_init(type, arr, capacity) dynarr_##type##_init(arr, capacity)
#define dynarr_append(type, arr, item) dynarr_##type##_append(arr, item)
#define dynarr_shrink(type, arr) dynarr_##type##_init(arr)

#define dynarr_lib_h(type) \
typedef struct dynarr_##type##_s {\
    type* base;\
    u64 len;\
    u64 cap;\
} dynarr_##type;\
void dynarr_##type##_init(dynarr(type)* arr, u64 capacity);\
void dynarr_##type##_append(dynarr(type)* arr, type item);\
void dynarr_##type##_shrink(dynarr(type)* arr);\

#define dynarr_lib(type) \
void dynarr_##type##_init(dynarr(type)* arr, u64 capacity) {\
    if (capacity == 0) capacity = 1;\
    *arr = (dynarr(type)){NULL, 0, capacity};\
    arr->base = (type*) malloc(sizeof(type) * capacity);\
}\
void dynarr_##type##_append(dynarr(type)* arr, type item) {\
    if (arr->len == arr->cap) {\
        arr->cap *= 1.75;\
        arr->base = (type*) realloc(arr->base, sizeof(type) * arr->cap);\
    }\
    arr->base[arr->len++] = item;\
}\
void dynarr_##type##_shrink(dynarr(type)* arr) {\
    arr->base = (type*) realloc(arr->base, sizeof(type) * arr->len);\
}\
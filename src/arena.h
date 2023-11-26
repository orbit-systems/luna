#pragma once
#define ARENA_H

#include "luna.h"

typedef struct arena_s {
    size_t size;
    char* base;
    char* current;
} arena;

void  arena_make(arena* a, size_t size);
void* arena_alloc(arena* a, size_t size, size_t align);
void  arena_free(arena* a);
void  arena_destroy(arena* a);

size_t align_forward(size_t ptr, size_t align);
size_t align_backward(size_t ptr, size_t align);

#define is_pow_2(i) ((i & (i-1)) == 0)
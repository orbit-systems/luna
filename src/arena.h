#pragma once
#define ARENA_H

#include "luna.h"

typedef struct arena_s {
    size_t size;
    char* base;
    char* current;
    bool  wrap; // when true, resets (frees) the allocator when it has reached capacity. this is good for temp allocators
} arena;

void  arena_make(arena* a, size_t size, bool wrap);
void* arena_alloc(arena* a, size_t size, size_t align);
void  arena_free(arena* a);
void  arena_destroy(arena* a);

size_t align_forward(size_t ptr, size_t align);
size_t align_backward(size_t ptr, size_t align);

#define is_pow_2(i) ((i & (i-1)) == 0)


// growable arena thing
// basically a list of arenas
typedef struct growable_arena_s {
    arena* base;
    size_t len;
    size_t arena_size;
} growable_arena;

void growable_arena_make(growable_arena* ac, size_t arena_size, size_t num_arenas);
void* growable_arena_alloc(growable_arena* ac, size_t size, size_t align);
void growable_arena_destroy(growable_arena* ac);

extern arena temp;
void init_temp_arena();
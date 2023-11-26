#include "luna.h"
#include "arena.h"


size_t align_forward (size_t ptr, size_t align) {
    if (!is_pow_2(align)) {
        crash("align_backward align is not a power of two got (%d)\n", align);
    }
    
    size_t p = ptr;
    size_t mod = p & (align - 1);
    if (mod != 0) {
        p += align - mod;
    }
    return p;
}
size_t align_backward(size_t ptr, size_t align) {
    if (!is_pow_2(align)) {
        crash("align_backward align is not a power of two got (%d)\n", align);
    }

    size_t p = ptr - align + 1;
    size_t mod = p & (align - 1);
    if (mod != 0) {
        p += align - mod;
    }
    return p;
}

void arena_make(arena* a, size_t size) {
    *a = (arena){size,NULL,NULL};
    a->base = malloc(size);
    a->current = a->base;
    if (a->base == NULL) {
        crash("arena_make malloc failed with size %d\n", size);
    }
}

void* arena_alloc(arena* a, size_t size, size_t align) {
    if (!is_pow_2(align)) {
        crash("arena_alloc align is not a power of two (got %d)\n", align);
    }

    a->current = (char*)align_forward((size_t)a->current, align) + size;
    return a->current - size;
}

void arena_free(arena* a) {
    a->current = a->base;
};

void arena_destroy(arena* a) {
    free(a->base);
    *a = (arena){};
}
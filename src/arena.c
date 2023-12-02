#include "luna.h"
#include "arena.h"

arena temp;

void init_temp_arena() {
    arena_make(&temp, 4096, true);
}

size_t align_forward(size_t ptr, size_t align) {
    if (!is_pow_2(align)) {
        crash("align_backward align is not a power of two (got %zu)\n", align);
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
        crash("align_backward align is not a power of two (got %zu)\n", align);
    }

    size_t p = ptr - align + 1;
    size_t mod = p & (align - 1);
    if (mod != 0) {
        p += align - mod;
    }
    return p;
}

void arena_make(arena* a, size_t size, bool wrap) {
    *a = (arena){size,NULL,NULL,wrap};
    a->base = malloc(size);
    a->current = a->base;
    if (a->base == NULL) {
        crash("arena_make malloc failed with size %zu\n", size);
    }
}

void* arena_alloc(arena* a, size_t size, size_t align) {
    if (!is_pow_2(align)) {
        crash("arena_alloc align is not a power of two (got %zu)\n", align);
    }

    char* new = (char*)align_forward((size_t)a->current, align);

    if ((size_t)new + size >= (size_t)(a->base) + a->size) {
        if (a->wrap) {
            arena_free(a);
            new = (char*)align_forward((size_t)a->current, align);
        } else return NULL;
    }
    a->current = new + size;
    return new;
}

void arena_free(arena* a) {
    a->current = a->base;
};

void arena_destroy(arena* a) {
    free(a->base);
    *a = (arena){};
}



void growable_arena_make(growable_arena* ac, size_t arena_size, size_t num_arenas) {
    if (num_arenas == 0) num_arenas = 1;
    *ac = (growable_arena){
        NULL, 
        num_arenas, 
        arena_size,
    };

    ac->base = (arena*) malloc(sizeof(arena)*num_arenas);
    if (ac->base == NULL) {
        crash("growable_arena_make failed initial malloc\n");
    }

    // allocate arenas
    for (int i = 0; i < num_arenas; i++) {
        arena_make(&(ac->base[i]), ac->arena_size, false);
    }
}

void* growable_arena_alloc(growable_arena* ac, size_t size, size_t align) {
    if (!is_pow_2(align))
        crash("growable_arena_alloc align is not a power of two (got %zu)\n", align);
    

    if (size > ac->arena_size)
        crash("growable_arena_alloc attempt to allocate size greater than arena block size\n")

    // try allocating on last arena
    void* ptr = arena_alloc(&(ac->base[ac->len-1]), size, align);
    
    // grow arena list if alloc fails
    if (ptr == NULL) {
        ac->len += 1;
        ac->base = (arena*) realloc(ac->base, sizeof(arena) * ac->len);
        if (ac->base == NULL) {
            crash("growable_arena_alloc internal realloc failed\n");
        }

        // initialize top arena
        arena_make(&(ac->base[ac->len-1]), ac->arena_size, false);

        // attempt to allocate space again
        ptr = arena_alloc(&(ac->base[ac->len-1]), size, align);
        if (ptr == NULL)
            crash("growable_arena_alloc failed for some reason\n");
    }
    return ptr;
}

void growable_arena_destroy(growable_arena* ac) {
    for (int i = 0; i < ac->len; i++) arena_destroy(&(ac->base[i]));
    free(ac->base);
    *ac = (growable_arena){};
}
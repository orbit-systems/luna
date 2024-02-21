#include "parser.h"

element* new_element(arena* restrict alloca, element_kind kind) {
    element* e = arena_alloc(alloca, sizeof(element), alignof(element));
    *e = (element){};
    e->kind = kind;
    return e;
}

symbol* symbol_find(string name, da(symbol)* restrict symtable) {
    FOR_URANGE(i, 0, symtable->len) {
        if (string_eq(symtable->at[i].name, name)) return &symtable->at[i];
    }
    return NULL;
}

symbol* symbol_find_or_create(string name, da(symbol)* restrict symtable) {
    symbol* sym = symbol_find(name, symtable);
    if (sym == NULL) {
        sym = &symtable->at[symtable->len];
        da_append(symtable, ((symbol){name, 0}));
    }
    return sym;
}

// modify symbol with real expanded name
void symbol_expandname(symbol* restrict sym, symbol* restrict last_nonlocal, arena* restrict alloca) {
    if (sym->name.raw[0] != '.') return;
    string newname;
    newname.raw = arena_alloc(alloca, sym->name.len + last_nonlocal->name.len, 1);
    newname.len = sym->name.len + last_nonlocal->name.len;
    FOR_RANGE(i, 0, last_nonlocal->name.len) newname.raw[i] = last_nonlocal->name.raw[i];
    FOR_RANGE(i, 0, sym->name.len) newname.raw[last_nonlocal->name.len + i] = sym->name.raw[i];
    sym->name = newname;
}
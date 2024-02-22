#include "parser.h"
#include "term.h"

#define current_token (f->tokens.at[f->current_tok])
#define advance_token (f->current_tok++)
#define error_at_token(p, token, message, ...) \
    error_at_string((p)->path, (p)->text, (token).text, \
    message __VA_OPT__(,) __VA_ARGS__)

#define current_eq(cstr) (string_eq(current_token.text, to_string(cstr)))

void parse_file(luna_file* f) {
    while (current_token.type != tt_EOF) {
        if (current_token.type == tt_newline) {
            advance_token;
            continue;
        }

        if (current_token.type != tt_identifier)
            error_at_token(f, current_token, "expected directive or instruction");

        // weed out some directives
        
        if current_eq("define") {
            advance_token;

            if (current_token.type != tt_identifier)
                error_at_token(f, current_token, "expected symbol name");
            
            advance_token;

        }
    }
}

element* new_element(arena* restrict alloca, element_kind kind) {
    element* e = arena_alloc(alloca, sizeof(element), alignof(element));
    *e = (element){.kind = kind};
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

// modify symbol with real symbol name
void symbol_expandname(symbol* restrict sym, symbol* restrict last_nonlocal, arena* restrict alloca) {
    if (sym->name.raw[0] != '.') return;
    string newname;
    newname.raw = arena_alloc(alloca, sym->name.len + last_nonlocal->name.len, 1);
    newname.len = sym->name.len + last_nonlocal->name.len;
    FOR_RANGE(i, 0, last_nonlocal->name.len) newname.raw[i] = last_nonlocal->name.raw[i];
    FOR_RANGE(i, 0, sym->name.len) newname.raw[last_nonlocal->name.len + i] = sym->name.raw[i];
    sym->name = newname;
}
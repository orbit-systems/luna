#pragma once
#define PARSER_H

#include "orbit.h"
#include "lexer.h"
#include "arena.h"
#include "aphel.h"

typedef struct symbol {
    string name;
    u64 value;
    bool defined;
} symbol;
da_typedef(symbol);

typedef u8 argument_kind; enum {
    ak_invalid,
    ak_symbol,
    ak_register,
    ak_int_literal,
    ak_char_literal,
    ak_f16_literal,
    ak_f32_literal,
    ak_f64_literal,
    ak_str_literal,
};

typedef struct argument {
    union {
        symbol*   symbol;
        aphel_reg as_reg;
        i64       as_int;
        u64       as_char;
        f64       as_f64;
        f32       as_f32;
        f16       as_f16;
        char*     as_str;
    };
    argument_kind kind;
} argument;
da_typedef(argument);

typedef u8 element_kind; enum {
    ek_invalid,
    ek_label,
    ek_instruction,
};

typedef struct element {
    token* start;
    token* end;
    union {
        struct {
            symbol* symbol;
        } label;
        struct {
            da(argument) args;
            u8 opcode;
            u8 func;
            u8 format;
        } instr;
    };
    element_kind kind;
} element;

typedef struct {
    element ** at;
    size_t len;
    size_t cap;
} element_list;

typedef struct luna_file {
    string path;
    string text;
    da(token)   tokens;
    da(symbol)  symtab;
    element_list instrs;

    u64 current_tok;
    symbol* last_nonlocal;
    
    arena str_alloca;
    arena elem_alloca;
} luna_file;

void parse_file(luna_file* restrict f);

i64 parse_regular_literal(luna_file* restrict f);

element* new_element(arena* restrict alloca, element_kind kind);

symbol* symbol_find(string name, da(symbol)* restrict symtable);
symbol* symbol_find_or_create(string name, da(symbol)* restrict symtable);
void expandlocalname(string* restrict sym, symbol* restrict last_nonlocal, arena* restrict alloca);

#define sign_extend(val, bitsize) ((u64)((i64)((u64)val << (64-bitsize)) >> (64-bitsize)))
#define zero_extend(val, bitsize) ((u64)((u64)((u64)val << (64-bitsize)) >> (64-bitsize)))

// figures out if a value can be losslessly compressed into a bitwidth integer
#define can_losslessly_signext(value, bitwidth) ((value) == sign_extend((value), (bitwidth)))
#define can_losslessly_zeroext(value, bitwidth) ((value) == zero_extend((value), (bitwidth)))
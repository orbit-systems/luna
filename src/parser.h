#pragma once
#define PARSER_H

#include "orbit.h"
#include "lexer.h"
#include "arena.h"

typedef struct symbol {
    string name;
    u64 value;
} symbol;
da_typedef(symbol);

typedef u8 aphel_reg; enum {
    r_rz = 0,
    r_ra = 1,
    r_rb = 2,
    r_rc = 3,
    r_rd = 4,
    r_re = 5,
    r_rf = 6,
    r_rg = 7,
    r_rh = 8,
    r_ri = 9,
    r_rj = 10,
    r_rk = 11,
    r_ip = 12,
    r_sp = 13,
    r_fp = 14,
    r_st = 15,
};

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
    ek_pseudoinstruction,
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
        } instr;
    };
    element_kind kind;
} element;

element* new_element(arena* restrict alloca, element_kind kind);

symbol* symbol_find(string name, da(symbol)* restrict symtable);
symbol* symbol_find_or_create(string name, da(symbol)* restrict symtable);
void    symbol_expandname(symbol* restrict sym, symbol* restrict last_nonlocal, arena* restrict alloca);

#define sign_extend(val, bitsize) ((u64)((i64)((u64)val << (64-bitsize)) >> (64-bitsize)))
#define zero_extend(val, bitsize) ((u64)((u64)((u64)val << (64-bitsize)) >> (64-bitsize)))

// figures out if a value can be losslessly compressed into a bitwidth integer
#define can_losslessly_signext(value, bitwidth) ((value) == sign_extend((value), (bitwidth)))
#define can_losslessly_zeroext(value, bitwidth) ((value) == zero_extend((value), (bitwidth)))
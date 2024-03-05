#pragma once
#define PARSER_H

#include "orbit.h"
#include "lexer.h"
#include "arena.h"
// #include "aphel.h"
#include "aphelion.h"

#define sign_extend(val, bitsize) ((u64)((i64)((u64)val << (64-bitsize)) >> (64-bitsize)))
#define zero_extend(val, bitsize) ((u64)((u64)((u64)val << (64-bitsize)) >> (64-bitsize)))

// figures out if a value can be losslessly compressed into a bitwidth integer
#define can_losslessly_signext(value, bitwidth) ((value) == sign_extend((value), (bitwidth)))
#define can_losslessly_zeroext(value, bitwidth) ((value) == zero_extend((value), (bitwidth)))


typedef struct symbol {
    string name;
    u64 value;
    bool is_defined : 1;
    bool is_label : 1;
} symbol;

typedef struct {
    symbol ** at;
    size_t len;
    size_t cap;
} symbol_table;

typedef u8 statement_type; enum {
    stmt_label,
    stmt_directive,
    stmt_instruction,
    stmt_macro,
    stmt_substream,
};

typedef struct statement {
        token* start;
        u8     len;

        statement_type type;

        union {
            symbol* label;
            
            struct {
                instr_template* templ; // what instruction even IS this, man?

            } instruction;
            
            struct {

            } macro;
            
            struct {

            } substream;
        } as;
} statement;

typedef struct luna_file {
    string       path;
    string       text;
} luna_file;

da_typedef(luna_file);

typedef struct unit {
    da(luna_file) files;
    da(token)    tokens;
    symbol_table symtab;
    // element_list elems;

    u64 current_tok;
    symbol* last_nonlocal;
    
    arena str_alloca;
    arena elem_alloca;
    
} unit;



void parse_file(unit* restrict f);
void check_definitions(unit* restrict f);

i64 parse_regular_literal(unit* restrict f);

symbol* symbol_find(unit* restrict f, string name);
symbol* symbol_find_or_create(unit* restrict f, string name);
void expand_local_sym(string* restrict sym, symbol* restrict last_nonlocal, arena* restrict alloca);

string string_lit_value(unit* restrict f);
i64 int_lit_value(unit* restrict f);
f64 float_lit_value(unit* restrict f);
i64 char_lit_value(unit* restrict f);
int ascii_to_digit_val(unit* restrict f, char c, u8 base);

#define str_from_tokens(start, end) ((string){(start).text.raw, (end).text.raw - (start).text.raw + (end).text.len})
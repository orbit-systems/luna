#include "luna.h"
#include "lexer.h"
#include "parser.h"
#include "arena.h"
#include "error.h"

dynarr_lib(section)
dynarr_lib(symbol)
dynarr_lib(info_entry)
dynarr_lib(reference)

void parser_init(parser* p, lexer_state* l) {

    // reset parser struct
    *p = (parser){};

    // init all the tables!!
    dynarr_init(section,    &p->section_table, 1);
    dynarr_init(symbol,     &p->symbol_table, 1);
    dynarr_init(info_entry, &p->info_table, 1);
    dynarr_init(reference,  &p->reference_table, 1);

    // give the parser the lexer's token buffer
    p->tokens = l->tokens;

    p->text = l->text;
    p->text_len = l->text_len;
    p->path = l->text_path;
}

void parser_start(parser* p) {

    skip_newlines(p);
    parse_directive(p, true);
    
}

void parse_directive(parser* p, bool maybe_period) {

    // skip possible period
    if (maybe_period && current_token(p).type == tt_period) 
        advance_token(p);

    if (current_token(p).type != tt_identifier)
        error_at_position(p->path, p->text, current_token(p).start, current_token(p).len,
            "expected directive, got %s", token_type_str[current_token(p).type]);
    
    if (tok_str_eq(p, current_token(p), "define")) {

        // syntax check

        advance_token(p);
        param first_param = parse_param(p);
        if (first_param.type != pt_symbol)
            error_at_position(p->path, p->text, current_token(p).start, current_token(p).len,
                "expected symbol, got %s", param_type_str[first_param.type]);

        advance_token(p);
        param second_param = parse_param(p);            
        if (second_param.type != pt_int)
            error_at_position(p->path, p->text, current_token(p).start, current_token(p).len,
                "expected scalar literal, got %s", param_type_str[first_param.type]);

        advance_token(p);
        if (current_token(p).type != tt_newline)
            error_at_position(p->path, p->text, current_token(p).start, current_token(p).len,
                "expected new line, got %s", token_type_str[current_token(p).type]);

        // process

        symbol* s = &p->symbol_table.base[first_param.symbol_index];

        // is this symbol already defined?
        if (s->is_defined)
            error_at_position(p->path, p->text, peek_token(p, -3).start, peek_token(p, -3).len, // i think -3 is the correct offset? i dont wanna test it lmao
                "symbol cannot be redefined");

        s->is_defined = true;
        s->ident = &p->text[peek_token(p, -2).start];
        s->ident_len = peek_token(p, -2).len;
        s->reloc = sr_absolute;
        
        
        
        



    } else
    if (tok_str_eq(p, current_token(p), "bind")) {
        
    } else
    if (tok_str_eq(p, current_token(p), "type")) {
        
    } else
    if (tok_str_eq(p, current_token(p), "size")) {
        
    } else
    if (tok_str_eq(p, current_token(p), "section")) {
        
    } else
    if (tok_str_eq(p, current_token(p), "blank")) {
        
    } else
    if (tok_str_eq(p, current_token(p), "info")) {
        
    } else {
        error_at_position(p->path, p->text, current_token(p).start, current_token(p).len,
            "unrecognized directive");
    }

}

// returns index of symbol OR -1 if symbol does not exist
u64 get_symbol(parser* p, char* ident, u16 ident_len) {
    for (int i = 0; i < p->symbol_table.len; i++) {
        // stupid bounded string equality
        if (p->symbol_table.base[i].ident_len == ident_len && strncmp(p->symbol_table.base[i].ident, ident, ident_len))
            return i;
        
    }
    return (u64) -1;
}

// returns index into symbol table (this will be the top index if the symbol did not previously exist)
u64 get_or_add_symbol(parser* p, char* ident, u16 ident_len) {

    // check if symbol exists through linear search of symbol table
    u64 sym = get_symbol(p, ident, ident_len);
    if (sym != (u64) -1) return sym;

    // create new symbol
    symbol s = (symbol){
        ident,
        0,
        0,
        ident_len,
        sb_undef,
        sr_undef,
        st_undef,
        false
    };

    dynarr_append(symbol, &(p->symbol_table), s);
    return p->symbol_table.len-1;
}

param parse_param(parser* p) {
    param par = (param){};
    
    token t = current_token(p);

    switch (t.type) {
    case tt_char_literal:
        TODO("parse_param - char literals");
        break;
    case tt_int_literal:
        par.token = p->current_token_index;
        par.type = pt_int;
        par.value_integer = parse_int_literal(p);
        break;
    case tt_string_literal:
        TODO("parse_param - string literals");
    case tt_identifier: {
        // check if the identifier is a register name
        int register_index = -1;
        for (int i = 0; i < 16; i++) {
            if (tok_str_eq(p, t, register_names[i])) {
                register_index = i;
                break;
            }
        }
        if (register_index != -1) {
            par.token = p->current_token_index;
            par.type = pt_reg;
            par.value_register = register_index;
            break;
        }

        // this is definitely a symbol!
        par.token = p->current_token_index;
        par.type = pt_symbol;
        par.symbol_index = get_or_add_symbol(p, &p->text[t.start], t.len);
        } break;
    default:
        error_at_position(p->path, p->text, current_token(p).start, current_token(p).len,
            "expected parameter, got %s",  token_type_str[current_token(p).type]);
    }

    return par;
}

// parse an integer literal
u64 parse_int_literal(parser* p) {
    if (current_token(p).type != tt_int_literal)
        crash("parse_int_value: expected token of tt_int_literal\n");

    token* t = &current_token(p);

    bool is_negative = (p->text[t->start] == '-');
    
    u64 value = 0;
    switch (p->text[t->start+is_negative]) {
    case '0':
        switch (p->text[t->start+is_negative+1]) {
        case 'x':
        case 'X':
            value = _parse_integer(p, t, 16, is_negative+2);
            break;
        case 'o':
        case 'O':
            value = _parse_integer(p, t, 8, is_negative+2);
            break;
        case 's': // seximal support because funny
        case 'S':
            value = _parse_integer(p, t, 6, is_negative+2);
            break;
        case 'b':
        case 'B':
            value = _parse_integer(p, t, 2, is_negative+2);
            break;
        case 'd':
        case 'D':
            value = _parse_integer(p, t, 10, is_negative+2);
            break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            value = _parse_integer(p, t, 10, is_negative+1);
            break;
        default:
            error_at_position(p->path, p->text, t->start+is_negative+1, t->len,
                "invalid base prefix \'%c\'",  p->text[t->start+is_negative+1]);
        }
        break;

    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':
        value = _parse_integer(p, t, 10, is_negative);
        break;

    default:
        // error_at_position(p->path, p->text, current_token(p).start, current_token(p).len,
        //     "invalid integer literal");
        crash("non-integer-literal was categorized as integer literal\n");
    }
    return is_negative ? -value : value;
}




bool _valid_digit(char c, u64 base) {
    switch (base) {
    case 2:  return valid_0b(c);
    case 8:  return valid_0o(c);
    case 10: return valid_0d(c);
    case 16: return valid_0x(c);
    default:
        crash("_valid_digit - invalid base (%lu)", base);
    }
}

u64 _parse_integer(parser* p, token* t, u64 base, bool scan_offset) {
    u64 value = 0;
    for (int i = scan_offset; i < t->len; i++) {
            char c = p->text[t->start + i];

            if (c == '_') 
                continue;
            
            if (!_valid_digit(c, base))
                error_at_position(p->path, p->text, t->start+i, 1,
                    "invalid base %lu digit \'%c\'", base, c);

            u64 char_value = c - '0';
            if (c >= 'a' && c <= 'f') char_value = c - 'a' + 1 + 10;
            if (c >= 'A' && c <= 'F') char_value = c - 'A' + 1 + 10;

            value *= base;
            value += char_value;
        }
    return value;
}


const char* register_names[] = {
    "rz",
    "ra",
    "rb",
    "rc",
    "rd",
    "re",
    "rf",
    "rg",
    "rh",
    "ri",
    "rj",
    "rk",
    "pc",
    "sp",
    "fp",
    "st",
};

const char* param_type_str[] = {
    "undefined",
    "register",
    "scalar literal",
    "string literal",
    "symbol name"
};
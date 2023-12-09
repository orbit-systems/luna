#include "luna.h"
#include "lexer.h"
#include "dynarr.h"
#include "arena.h"

typedef u8 param_type; enum {
    pt_undef,
    pt_reg,
    pt_int,
    pt_str,
    pt_symbol,
};

extern const char* param_type_str[];

typedef struct param_s {
    union {
        u8    value_register;
        u64   value_integer;
        char* value_string;
        u32   value_char;
        u64   symbol_index;
    };
    u32 token;
    param_type type;
} param; 

dynarr_lib_h(u8)

typedef struct section_s {
    dynarr(u8) body; // byte buffer containing the section's actual stuff
    u64        base; // starting address of the section
    bool       perms_read;
    bool       perms_write;
    bool       perms_exec;
    bool       is_blank;
} section;

typedef u8 sym_bind; enum {
    sb_undef,
    sb_local,
    sb_global,
    sb_weak,
};

typedef u8 sym_reloc; enum {
    sr_undef,
    sr_relative,
    sr_absolute,
};

typedef u8 sym_type; enum {
    st_undef,
    st_untyped,
    st_function,
    st_data,
};

typedef struct symbol_s {
    char*     ident;            // symbol identifier
    u64       value;            // symbol value
    u32       section_index;    // section where the symbol is defined
    u16       ident_len;
    sym_bind  bind;             // visibility
    sym_reloc reloc;            // how to relocate
    sym_type  type;             // type of symbol
    bool      is_defined;       // is the symbol defined in?
} symbol;

typedef u8 ref_width; enum {
    rw_undef,
    rw_8bit,
    rw_12bit,
    rw_16bit,
    rw_20bit,   // for branches
    rw_32bit,
    rw_64bit,
};

typedef struct reference_s {
    u64       location;         // where the reference is, relative from associated section's base
    u64       symbol_index;     // what symbol is it referencing?
    i64       const_offset;     // what value to add to the symbol
    u32       section_index;    // what section is the reference in?
    ref_width ref_width;        // bit width of the reference
    u8        shift_right;      // how much to arithmetic-shift-right the value before replacing it? useful for LI, JAL, and stuff
    bool      rel_to_position;  // should the value of the symbol be subtracted by the position of the reference?
} reference;
// final value of a reference = (i64)(symbol.value + const_offset) >> shift_right


typedef struct info_entry_s {
    char* key, val;
} info_entry;

dynarr_lib_h(section)
dynarr_lib_h(symbol)
dynarr_lib_h(info_entry)
dynarr_lib_h(reference)

typedef struct parser_s {
    dynarr(section)    section_table;
    dynarr(symbol)     symbol_table;
    dynarr(info_entry) info_table;
    dynarr(reference)  reference_table;

    dynarr(token) tokens;
    u64 current_token_index;

    growable_arena string_crap; // allocator for parsed strings

    char* text;
    u64   text_len;
    char* path;

} parser;

#define current_token(p) (p->tokens.base[p->current_token_index])
#define advance_token(p) if (p->current_token_index < p->tokens.len) p->current_token_index++
#define peek_token(p, amnt) (p->tokens.base[p->current_token_index + amnt])
// TODO ^^^ make this safer probably!

#define tok_str_eq(p, token, string) (strlen(string) == token.len && strncmp(p->text + token.start, string, token.len) == 0)

#define skip_newlines(p) while (current_token(p).type == tt_newline) advance_token(p)

void parser_init(parser* p, lexer_state* l);
void parser_start(parser* p);
void parse_directive(parser* p, bool maybe_period);
param parse_param(parser* p);
u64 parse_int_literal(parser* p);

u64 _parse_integer(parser* p, token* t, u64 base, bool is_negative);

extern const char* register_names[16];
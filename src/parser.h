#include "luna.h"
#include "lexer.h"

typedef u8 param_type; enum {
    pt_reg,
    pt_int,
    pt_str,
    pt_char,
    pt_symbol,
};

typedef struct param_s {
    union {
        u8    value_register;
        u64   value_integer;
        char* value_string;
        u32   value_char;
    } as;
    u32 token;
    param_type type;
} param;

typedef u8 entity_type; enum {
    et_instruction,
    et_label,
};

typedef struct entity_s {
    u32 start, end; // first and last tokens in the
    entity_type type;
    dynarr(param) params;
} entity;
#include "luna.h"
#include "lexer.h"
#include "dynarr.h"

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

dynarr_lib_h(param)

typedef u8 entity_type; enum {
    et_instruction,
    et_label,
    et_directive,
    et_section,
    et_metadata,
};

typedef struct entity_s {
    u32 start, end; // first and last tokens encompassing the statement
    entity_type type;
    dynarr(param) params;
} entity;
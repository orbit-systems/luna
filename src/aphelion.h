#pragma once
#define APHELION_H

#include "orbit.h"

typedef u8 aph_register; enum {
    reg_rz = 0,
    reg_ra = 1,
    reg_rb = 2,
    reg_rc = 3,
    reg_rd = 4,
    reg_re = 5,
    reg_rf = 6,
    reg_rg = 7,
    reg_rh = 8,
    reg_ri = 9,
    reg_rj = 10,
    reg_rk = 11,
    reg_ip = 12,
    reg_sp = 13,
    reg_fp = 14,
    reg_st = 15,
};

typedef u8 aph_field; enum {
    field_invalid,
    field_op,
    field_rde,
    field_rs1,
    field_rs2,
    field_imm,
    field_func,
};

typedef u8 aph_fmt; enum {
    fmt_invalid,
    fmt_E,
    fmt_R,
    fmt_M,
    fmt_F,
    fmt_B,
};

typedef u8 arg_type; enum {
    arg_invalid,
    arg_symbol,
    arg_val,
    arg_val_or_sym,
    arg_string,
    arg_register,
};

typedef u8 special_handling; enum {
    special_branch,
    special_jumplink,
    special_imm8,
};

typedef struct instr_template {
    string name;

    arg_type* args;
    aph_field* fields;
    u8 count;

    u8 opcode;
    u8 func : 4;
    aph_fmt format : 4;

    u8 special_handling;
} instr_template;

#define TEMPLATES_LEN 8

extern instr_template templates[];

// map multiple templates to a single name
// selects templates based on argument count/types
typedef struct template_group {
    string name;
    instr_template** matches;
    u64 count;
} template_group;

extern template_group templ_groups[];


// macros that get special handling by the luna core. 
// These macros cant be reduced to other functionality
// things like call and li.
typedef struct macro_template {
    string name;

} macro_template;
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

typedef struct instr_template {
    string name;

    arg_type* args;
    aph_field* fields;
    u8 count;

    u8 opcode;

    u8 func;
    aph_fmt format;
} instr_template;

#define TEMPLATES_LEN 8

extern const instr_template templates[];
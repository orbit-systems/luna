#include "aphelion.h"

// constant cstr -> string conversion
#define constr(cstr) (string){cstr, sizeof(cstr) - sizeof(char)}

instr_template templates[] = {
    {constr("int"),
        .args   = (arg_type[]) {arg_val_or_sym},
        .fields = (aph_field[]){field_imm},
        .count  = 1,
        .opcode = 0x01,
        .func   = 0x00,
        .format = fmt_F},
    {constr("iret"),
        .args   = NULL,
        .fields = NULL,
        .count  = 0,
        .opcode = 0x01,
        .func   = 0x01,
        .format = fmt_F},
    {constr("ires"),
        .args   = NULL,
        .fields = NULL,
        .count  = 0,
        .opcode = 0x01,
        .func   = 0x02,
        .format = fmt_F},
    {constr("usr"),
        .args   = (arg_type[]) {arg_register},
        .fields = (aph_field[]){field_rde},
        .count  = 1,
        .opcode = 0x01,
        .func   = 0x03,
        .format = fmt_F},
    

    {constr("outr"),
        .args   = (arg_type[]) {arg_register, arg_register},
        .fields = (aph_field[]){field_rde,    field_rs1},
        .count  = 2,
        .opcode = 0x02,
        .func   = 0x00,
        .format = fmt_M},
    {constr("outi"),
        .args   = (arg_type[]) {arg_val_or_sym, arg_register},
        .fields = (aph_field[]){field_imm,      field_rs1},
        .count  = 2,
        .opcode = 0x03,
        .func   = 0x00,
        .format = fmt_M},
    {constr("inr"),
        .args   = (arg_type[]) {arg_register, arg_register},
        .fields = (aph_field[]){field_rde,    field_rs1},
        .count  = 2,
        .opcode = 0x04,
        .func   = 0x00,
        .format = fmt_M},
    {constr("ini"),
        .args   = (arg_type[]) {arg_register, arg_val_or_sym},
        .fields = (aph_field[]){field_rde,    field_imm},
        .count  = 2,
        .opcode = 0x05,
        .func   = 0x00,
        .format = fmt_M},
};

static_assert(TEMPLATES_LEN == (sizeof(templates) / sizeof(templates[0])), "TEMPLATES_LEN must be updated to reflect the actual template array length");

template_group templ_groups[] = {
    {constr("out"),
        .matches = (instr_template*[]){&templates[4], &templates[5]},
        .count   = 2 },
    {constr("in"),
        .matches = (instr_template*[]){&templates[6], &templates[7]},
        .count   = 2 },
};

static_assert(TEMPL_GROUPS_LEN == (sizeof(templ_groups) / sizeof(templ_groups[0])), "TEMPL_GROUPS_LEN must be updated to reflect the actual template array length");
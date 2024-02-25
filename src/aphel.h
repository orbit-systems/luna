#pragma once
#define APHEL_H

#include "orbit.h"

typedef u8 aphel_reg; enum {
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

typedef u8 aphel_fmt; enum {
    fmt_invalid,
    fmt_E,
    fmt_R,
    fmt_M,
    fmt_F,
    fmt_B,
};

// how to treat and check imm values
typedef u8 imm_strategy; enum {
    imm_none,
    imm_zeroext_8,
    imm_zeroext_16,
    imm_zeroext_20,
    imm_signext_8,
    imm_signext_16,
    imm_signext_32,
    imm_branch,
};

// name, opcode, func, format
#define INSTRUCTION_LIST \
    INSTR(REAL_MIN, "",   0x00, 0, 0     ) \
    INSTR(int,   "int",   0x01, 0, fmt_F ) \
    INSTR(iret,  "iret",  0x01, 1, fmt_F ) \
    INSTR(ires,  "ires",  0x01, 2, fmt_F ) \
    INSTR(usr,   "usr",   0x01, 3, fmt_F ) \
    \
    INSTR(outr,  "outr",  0x02, 0, fmt_M ) \
    INSTR(outi,  "outi",  0x03, 0, fmt_M ) \
    INSTR(inr,   "inr",   0x04, 0, fmt_M ) \
    INSTR(ini,   "ini",   0x05, 0, fmt_M ) \
    \
    INSTR(jal,   "jal",   0x06, 0, fmt_M ) \
    INSTR(jalr,  "jalr",  0x07, 0, fmt_M ) \
    INSTR(ret,   "ret",   0x08, 0, fmt_M ) \
    INSTR(retr,  "retr",  0x09, 0, fmt_M ) \
    INSTR(bra,   "bra",   0x0a, 0, fmt_B ) \
    INSTR(beq,   "beq",   0x0a, 1, fmt_B ) \
    INSTR(bez,   "bez",   0x0a, 2, fmt_B ) \
    INSTR(blt,   "blt",   0x0a, 3, fmt_B ) \
    INSTR(ble,   "ble",   0x0a, 4, fmt_B ) \
    INSTR(bltu,  "bltu",  0x0a, 5, fmt_B ) \
    INSTR(bleu,  "bleu",  0x0a, 6, fmt_B ) \
    INSTR(bne,   "bne",   0x0a, 9, fmt_B ) \
    INSTR(bnz,   "bnz",   0x0a, 10, fmt_B) \
    INSTR(bge,   "bge",   0x0a, 11, fmt_B) \
    INSTR(bgt,   "bgt",   0x0a, 12, fmt_B) \
    INSTR(bgeu,  "bgeu",  0x0a, 13, fmt_B) \
    INSTR(bgtu,  "bgtu",  0x0a, 14, fmt_B) \
    \
    INSTR(push,  "push",  0x0b, 0, fmt_M ) \
    INSTR(pop,   "pop",   0x0c, 0, fmt_M ) \
    INSTR(enter, "enter", 0x0d, 0, fmt_B ) \
    INSTR(leave, "leave", 0x0e, 0, fmt_B ) \
    \
    INSTR(lli,   "lli",   0x10, 0, fmt_F ) \
    INSTR(llis,  "llis",  0x10, 1, fmt_F ) \
    INSTR(lui,   "lui",   0x10, 2, fmt_F ) \
    INSTR(luis,  "luis",  0x10, 3, fmt_F ) \
    INSTR(lti,   "lti",   0x10, 4, fmt_F ) \
    INSTR(ltis,  "ltis",  0x10, 5, fmt_F ) \
    INSTR(ltui,  "ltui",  0x10, 6, fmt_F ) \
    INSTR(ltuis, "ltuis", 0x10, 7, fmt_F ) \
    INSTR(lw,    "lw",    0x11, 0, fmt_E ) \
    INSTR(lh,    "lh",    0x12, 0, fmt_E ) \
    INSTR(lhs,   "lhs",   0x13, 0, fmt_E ) \
    INSTR(lq,    "lq",    0x14, 0, fmt_E ) \
    INSTR(lqs,   "lqs",   0x15, 0, fmt_E ) \
    INSTR(lb,    "lb",    0x16, 0, fmt_E ) \
    INSTR(lbs,   "lbs",   0x17, 0, fmt_E ) \
    INSTR(sw,    "sw",    0x18, 0, fmt_E ) \
    INSTR(sh,    "sh",    0x19, 0, fmt_E ) \
    INSTR(sq,    "sq",    0x1a, 0, fmt_E ) \
    INSTR(sb,    "sb",    0x1b, 0, fmt_E ) \
    \
    INSTR(cmpr,  "cmpr",  0x1e, 0, fmt_M ) \
    INSTR(cmpi,  "cmpi",  0x1f, 0, fmt_F ) \
    \
    INSTR(addr,  "addr",  0x20, 0, fmt_R ) \
    INSTR(addi,  "addi",  0x21, 0, fmt_M ) \
    INSTR(subr,  "subr",  0x22, 0, fmt_R ) \
    INSTR(subi,  "subi",  0x23, 0, fmt_M ) \
    INSTR(imulr, "imulr", 0x24, 0, fmt_R ) \
    INSTR(imuli, "imuli", 0x25, 0, fmt_M ) \
    INSTR(idivr, "idivr", 0x26, 0, fmt_R ) \
    INSTR(idivi, "idivi", 0x27, 0, fmt_M ) \
    INSTR(umulr, "umulr", 0x28, 0, fmt_R ) \
    INSTR(umuli, "umuli", 0x29, 0, fmt_M ) \
    INSTR(udivr, "udivr", 0x2a, 0, fmt_R ) \
    INSTR(udivi, "udivi", 0x2b, 0, fmt_M ) \
    INSTR(remr,  "remr",  0x2c, 0, fmt_R ) \
    INSTR(remi,  "remi",  0x2d, 0, fmt_M ) \
    INSTR(modr,  "modr",  0x2e, 0, fmt_R ) \
    INSTR(modi,  "modi",  0x2f, 0, fmt_M ) \
    \
    INSTR(andr,  "andr",  0x30, 0, fmt_R ) \
    INSTR(andi,  "andi",  0x31, 0, fmt_M ) \
    INSTR(orr,   "orr",   0x32, 0, fmt_R ) \
    INSTR(ori,   "ori",   0x33, 0, fmt_M ) \
    INSTR(norr,  "norr",  0x34, 0, fmt_R ) \
    INSTR(nori,  "nori",  0x35, 0, fmt_M ) \
    INSTR(xorr,  "xorr",  0x36, 0, fmt_R ) \
    INSTR(xori,  "xori",  0x37, 0, fmt_M ) \
    INSTR(shlr,  "shlr",  0x38, 0, fmt_R ) \
    INSTR(shli,  "shli",  0x39, 0, fmt_M ) \
    INSTR(asrr,  "asrr",  0x3a, 0, fmt_R ) \
    INSTR(asri,  "asri",  0x3b, 0, fmt_M ) \
    INSTR(lsrr,  "lsrr",  0x3c, 0, fmt_R ) \
    INSTR(lsri,  "lsri",  0x3d, 0, fmt_M ) \
    INSTR(bitr,  "bitr",  0x3e, 0, fmt_R ) \
    INSTR(biti,  "biti",  0x3f, 0, fmt_M ) \
    \
    INSTR(fcmp16,  "fcmp.16",  0x40, 0, fmt_E) \
    INSTR(fto16,   "fto.16",   0x41, 0, fmt_E) \
    INSTR(ffrom16, "ffrom.16", 0x42, 0, fmt_E) \
    INSTR(fneg16,  "fneg.16",  0x43, 0, fmt_E) \
    INSTR(fabs16,  "fabs.16",  0x44, 0, fmt_E) \
    INSTR(fadd16,  "fadd.16",  0x45, 0, fmt_E) \
    INSTR(fsub16,  "fsub.16",  0x46, 0, fmt_E) \
    INSTR(fmul16,  "fmul.16",  0x47, 0, fmt_E) \
    INSTR(fdiv16,  "fdiv.16",  0x48, 0, fmt_E) \
    INSTR(fma16,   "fma.16",   0x49, 0, fmt_E) \
    INSTR(fsqrt16, "fsqrt.16", 0x4a, 0, fmt_E) \
    INSTR(fmin16,  "fmin.16",  0x4b, 0, fmt_E) \
    INSTR(fmax16,  "fmax.16",  0x4c, 0, fmt_E) \
    INSTR(fsat16,  "fsat.16",  0x4d, 0, fmt_E) \
    INSTR(fcnv16_32,  "fcnv.16.32",  0x4e, 0b0100, fmt_E) \
    INSTR(fcnv16_64,  "fcnv.16.64",  0x4e, 0b1000, fmt_E) \
    INSTR(fnan16,  "fnan.16",  0x4f, 0, fmt_E) \
\
    INSTR(fcmp32,  "fcmp.32",  0x40, 1, fmt_E) \
    INSTR(fto32,   "fto.32",   0x41, 1, fmt_E) \
    INSTR(ffrom32, "ffrom.32", 0x42, 1, fmt_E) \
    INSTR(fneg32,  "fneg.32",  0x43, 1, fmt_E) \
    INSTR(fabs32,  "fabs.32",  0x44, 1, fmt_E) \
    INSTR(fadd32,  "fadd.32",  0x45, 1, fmt_E) \
    INSTR(fsub32,  "fsub.32",  0x46, 1, fmt_E) \
    INSTR(fmul32,  "fmul.32",  0x47, 1, fmt_E) \
    INSTR(fdiv32,  "fdiv.32",  0x48, 1, fmt_E) \
    INSTR(fma32,   "fma.32",   0x49, 1, fmt_E) \
    INSTR(fsqrt32, "fsqrt.32", 0x4a, 1, fmt_E) \
    INSTR(fmin32,  "fmin.32",  0x4b, 1, fmt_E) \
    INSTR(fmax32,  "fmax.32",  0x4c, 1, fmt_E) \
    INSTR(fsat32,  "fsat.32",  0x4d, 1, fmt_E) \
    INSTR(fcnv32_16,  "fcnv.32.16",  0x4e, 0b0001, fmt_E) \
    INSTR(fcnv32_64,  "fcnv.32.64",  0x4e, 0b1001, fmt_E) \
    INSTR(fnan32,  "fnan.32",  0x4f, 1, fmt_E) \
\
    INSTR(fcmp64,  "fcmp.64",  0x40, 2, fmt_E) \
    INSTR(fto64,   "fto.64",   0x41, 2, fmt_E) \
    INSTR(ffrom64, "ffrom.64", 0x42, 2, fmt_E) \
    INSTR(fneg64,  "fneg.64",  0x43, 2, fmt_E) \
    INSTR(fabs64,  "fabs.64",  0x44, 2, fmt_E) \
    INSTR(fadd64,  "fadd.64",  0x45, 2, fmt_E) \
    INSTR(fsub64,  "fsub.64",  0x46, 2, fmt_E) \
    INSTR(fmul64,  "fmul.64",  0x47, 2, fmt_E) \
    INSTR(fdiv64,  "fdiv.64",  0x48, 2, fmt_E) \
    INSTR(fma64,   "fma.64",   0x49, 2, fmt_E) \
    INSTR(fsqrt64, "fsqrt.64", 0x4a, 2, fmt_E) \
    INSTR(fmin64,  "fmin.64",  0x4b, 2, fmt_E) \
    INSTR(fmax64,  "fmax.64",  0x4c, 2, fmt_E) \
    INSTR(fsat64,  "fsat.64",  0x4d, 2, fmt_E) \
    INSTR(fcnv64_16,  "fcnv.64.16",  0x4e, 0b0010, fmt_E) \
    INSTR(fcnv64_32,  "fcnv.64.32",  0x4e, 0b0110, fmt_E) \
    INSTR(fnan64,  "fnan.64",  0x4f, 2, fmt_E) \
\
    INSTR(REAL_MAX, "",       0x00, 0, 0) \
\
    INSTR(PSUEDO_MIN, "",     0x00, 0, 0) \
\
    INSTR(nop,     "nop",     0x00, 0, 0) \
    INSTR(inv,     "inv",     0x00, 0, 0) \
    INSTR(in,      "in",      0x00, 0, 0) \
    INSTR(out,     "out",     0x00, 0, 0) \
    INSTR(call,    "call",    0x00, 0, 0) \
    INSTR(callr,   "callr",   0x00, 0, 0) \
    INSTR(mov,     "mov",     0x00, 0, 0) \
    INSTR(li,      "li",      0x00, 0, 0) \
    INSTR(cmp,     "cmp",     0x00, 0, 0) \
    INSTR(add,     "add",     0x00, 0, 0) \
    INSTR(sub,     "sub",     0x00, 0, 0) \
    INSTR(imul,    "imul",    0x00, 0, 0) \
    INSTR(idiv,    "idiv",    0x00, 0, 0) \
    INSTR(umul,    "umul",    0x00, 0, 0) \
    INSTR(udiv,    "udiv",    0x00, 0, 0) \
    INSTR(mod,     "mod",     0x00, 0, 0) \
    INSTR(rem,     "rem",     0x00, 0, 0) \
    INSTR(and,     "and",     0x00, 0, 0) \
    INSTR(or,      "or",      0x00, 0, 0) \
    INSTR(nor,     "nor",     0x00, 0, 0) \
    INSTR(not,     "not",     0x00, 0, 0) \
    INSTR(xor,     "xor",     0x00, 0, 0) \
    INSTR(shl,     "shl",     0x00, 0, 0) \
    INSTR(asr,     "asr",     0x00, 0, 0) \
    INSTR(lsr,     "lsr",     0x00, 0, 0) \
    INSTR(bit,     "bit",     0x00, 0, 0) \
    INSTR(setfs,   "setfs",   0x00, 0, 0) \
    INSTR(setfz,   "setfz",   0x00, 0, 0) \
    INSTR(setfcb,  "setfcb",  0x00, 0, 0) \
    INSTR(setfcbu, "setfcbu", 0x00, 0, 0) \
    INSTR(setfe,   "setfe",   0x00, 0, 0) \
    INSTR(setfl,   "setfl",   0x00, 0, 0) \
    INSTR(setflu,  "setflu",  0x00, 0, 0) \
\
    INSTR(loc,   "loc",   0x00, 0, 0) \
    INSTR(align, "align", 0x00, 0, 0) \
    INSTR(skip,  "skip",  0x00, 0, 0) \
    INSTR(byte,  "byte",  0x00, 0, 0) \
    INSTR(d8,    "d8",    0x00, 0, 0) \
    INSTR(d16,   "d16",   0x00, 0, 0) \
    INSTR(d32,   "d32",   0x00, 0, 0) \
    INSTR(d64,   "d64",   0x00, 0, 0) \
    INSTR(utf8,  "utf8",  0x00, 0, 0) \
\
    INSTR(PSUEDO_MAX, "",  0x00, 0, 0) \


typedef u16 aphel_inst_code; enum {
    aphel_invalid,
#   define INSTR(name_, namestreg_, opcode_, func_, format_) aphel_##name_,
        INSTRUCTION_LIST
#   undef INSTR
    aphel_COUNT
};

enum {
    special_none,
    special_cmpi_reverse,
};

typedef union {
    u8 opcode;
    struct {
        u32 opcode : 8;
        u32 imm    : 8;
        u32 func   : 4;
        u32 rs2    : 4;
        u32 rs1    : 4;
        u32 rde    : 4;
    } E;
    struct {
        u32 opcode : 8;
        u32 imm    : 12;
        u32 rs2    : 4;
        u32 rs1    : 4;
        u32 rde    : 4;
    } R;
    struct {
        u32 opcode : 8;
        u32 imm    : 16;
        u32 rs1    : 4;
        u32 rde    : 4;
    } M;
    struct {
        u32 opcode : 8;
        u32 imm    : 16;
        u32 func   : 4;
        u32 rde    : 4;
    } F;
    struct {
        u32 opcode : 8;
        u32 imm    : 20;
        u32 func   : 4;
    } B;
} aphel_instr;
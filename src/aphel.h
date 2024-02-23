#pragma once
#define APHEL_H

#include "orbit.h"

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

typedef u8 aphel_fmt; enum {
    fmt_E,
    fmt_R,
    fmt_M,
    fmt_F,
    fmt_B,
};

typedef u8 aphel_imm; enum {
    imm_none,
    imm_interrupt,
    imm_signext,
    imm_zeroext,
    imm_branch,
};

// name, opcode, func, format
#define INSTRUCTION_LIST \
    INSTR(int,   "int",   0x01, 0, fmt_F) \
    INSTR(iret,  "iret",  0x01, 1, fmt_F) \
    INSTR(ires,  "ires",  0x01, 2, fmt_F) \
    INSTR(usr,   "usr",   0x01, 3, fmt_F) \
    \
    INSTR(outr,  "outr",  0x02, 0, fmt_M) \
    INSTR(outi,  "outi",  0x03, 0, fmt_M) \
    INSTR(inr,   "inr",   0x04, 0, fmt_M) \
    INSTR(ini,   "ini",   0x05, 0, fmt_M) \
    \
    INSTR(jal,   "jal",   0x06, 0, fmt_M) \
    INSTR(jalr,  "jalr",  0x07, 0, fmt_M) \
    INSTR(ret,   "ret",   0x08, 0, fmt_M) \
    INSTR(retr,  "retr",  0x09, 0, fmt_M) \
    INSTR(bra,   "bra",   0x0a, 0, fmt_B) \
    INSTR(beq,   "beq",   0x0a, 1, fmt_B) \
    INSTR(bez,   "bez",   0x0a, 2, fmt_B) \
    INSTR(blt,   "blt",   0x0a, 3, fmt_B) \
    INSTR(ble,   "ble",   0x0a, 4, fmt_B) \
    INSTR(bltu,  "bltu",  0x0a, 5, fmt_B) \
    INSTR(bleu,  "bleu",  0x0a, 6, fmt_B) \
    INSTR(bne,   "bne",   0x0a, 9, fmt_B) \
    INSTR(bnz,   "bnz",   0x0a, 10, fmt_B) \
    INSTR(bge,   "bge",   0x0a, 11, fmt_B) \
    INSTR(bgt,   "bgt",   0x0a, 12, fmt_B) \
    INSTR(bgeu,  "bgeu",  0x0a, 13, fmt_B) \
    INSTR(bgtu,  "bgtu",  0x0a, 14, fmt_B) \
    \
    INSTR(push,  "push",  0x0b, 0, fmt_M) \
    INSTR(pop,   "pop",   0x0c, 0, fmt_M) \
    INSTR(enter, "enter", 0x0d, 0, fmt_B) \
    INSTR(leave, "leave", 0x0e, 0, fmt_B) \
    \
    INSTR(lli,   "lli",   0x10, 0, fmt_F) \
    INSTR(llis,  "llis",  0x10, 1, fmt_F) \
    INSTR(lui,   "lui",   0x10, 2, fmt_F) \
    INSTR(luis,  "luis",  0x10, 3, fmt_F) \
    INSTR(lti,   "lti",   0x10, 4, fmt_F) \
    INSTR(ltis,  "ltis",  0x10, 5, fmt_F) \
    INSTR(ltui,  "ltui",  0x10, 6, fmt_F) \
    INSTR(ltuis, "ltuis", 0x10, 7, fmt_F) \
    INSTR(lw,    "lw",    0x11, 0, fmt_E) \
    INSTR(lh,    "lh",    0x12, 0, fmt_E) \
    INSTR(lhs,   "lhs",   0x13, 0, fmt_E) \
    INSTR(lq,    "lq",    0x14, 0, fmt_E) \
    INSTR(lqs,   "lqs",   0x15, 0, fmt_E) \
    INSTR(lb,    "lb",    0x16, 0, fmt_E) \
    INSTR(lbs,   "lbs",   0x17, 0, fmt_E) \
    INSTR(sw,    "sw",    0x18, 0, fmt_E) \
    INSTR(sh,    "sh",    0x19, 0, fmt_E) \
    INSTR(sq,    "sq",    0x1a, 0, fmt_E) \
    INSTR(sb,    "sb",    0x1b, 0, fmt_E) \
    \
    INSTR(cmpr,  "cmpr",  0x1e, 0, fmt_M) \
    INSTR(cmpi,  "cmpi",  0x1f, 0, fmt_F) \
    \
    INSTR(addr,  "addr",  0x20, 0, fmt_R) \
    INSTR(addi,  "addi",  0x21, 0, fmt_M) \
    INSTR(subr,  "subr",  0x22, 0, fmt_R) \
    INSTR(subi,  "subi",  0x23, 0, fmt_M) \
    INSTR(imulr, "imulr", 0x24, 0, fmt_R) \
    INSTR(imuli, "imuli", 0x25, 0, fmt_M) \
    INSTR(idivr, "idivr", 0x26, 0, fmt_R) \
    INSTR(idivi, "idivi", 0x27, 0, fmt_M) \
    INSTR(umulr, "umulr", 0x28, 0, fmt_R) \
    INSTR(umuli, "umuli", 0x29, 0, fmt_M) \
    INSTR(udivr, "udivr", 0x2a, 0, fmt_R) \
    INSTR(udivi, "udivi", 0x2b, 0, fmt_M) \
    INSTR(remr,  "remr",  0x2c, 0, fmt_R) \
    INSTR(remi,  "remi",  0x2d, 0, fmt_M) \
    INSTR(modr,  "modr",  0x2e, 0, fmt_R) \
    INSTR(modi,  "modi",  0x2f, 0, fmt_M) \
    \
    INSTR(andr,  "andr",  0x30, 0, fmt_R) \
    INSTR(andi,  "andi",  0x31, 0, fmt_M) \
    INSTR(orr,   "orr",   0x32, 0, fmt_R) \
    INSTR(ori,   "ori",   0x33, 0, fmt_M) \
    INSTR(norr,  "norr",  0x34, 0, fmt_R) \
    INSTR(nori,  "nori",  0x35, 0, fmt_M) \
    INSTR(xorr,  "xorr",  0x36, 0, fmt_R) \
    INSTR(xori,  "xori",  0x37, 0, fmt_M) \
    INSTR(shlr,  "shlr",  0x38, 0, fmt_R) \
    INSTR(shli,  "shli",  0x39, 0, fmt_M) \
    INSTR(asrr,  "asrr",  0x3a, 0, fmt_R) \
    INSTR(asri,  "asri",  0x3b, 0, fmt_M) \
    INSTR(lsrr,  "lsrr",  0x3c, 0, fmt_R) \
    INSTR(lsri,  "lsri",  0x3d, 0, fmt_M) \
    INSTR(bitr,  "bitr",  0x3e, 0, fmt_R) \
    INSTR(biti,  "biti",  0x3f, 0, fmt_M) \
    \
    INSTR(fcmp,  "fcmp",  0x40, 0, fmt_E) \
    INSTR(fto,   "fto",   0x41, 0, fmt_E) \
    INSTR(ffrom, "ffrom", 0x42, 0, fmt_E) \
    INSTR(fneg,  "fneg",  0x43, 0, fmt_E) \
    INSTR(fabs,  "fabs",  0x44, 0, fmt_E) \
    INSTR(fadd,  "fadd",  0x45, 0, fmt_E) \
    INSTR(fsub,  "fsub",  0x46, 0, fmt_E) \
    INSTR(fmul,  "fmul",  0x47, 0, fmt_E) \
    INSTR(fdiv,  "fdiv",  0x48, 0, fmt_E) \
    INSTR(fma,   "fma",   0x49, 0, fmt_E) \
    INSTR(fsqrt, "fsqrt", 0x4a, 0, fmt_E) \
    INSTR(fmin,  "fmin",  0x4b, 0, fmt_E) \
    INSTR(fmax,  "fmax",  0x4c, 0, fmt_E) \
    INSTR(fsat,  "fsat",  0x4d, 0, fmt_E) \
    INSTR(fcnv,  "fcnv",  0x4e, 0, fmt_E) \
    INSTR(fnan,  "fnan",  0x4f, 0, fmt_E) \

typedef u16 aphel_inst_code; enum {
    aphel_invalid,
#   define INSTR(name_, namestr_, opcode_, func_, format_) aphel_##name_,
        INSTRUCTION_LIST
#   undef INSTR
    aphel_COUNT
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

typedef u8 aphel_pseudo_code; enum {
    aphel_p_loc,
    aphel_p_align,
    aphel_p_d8,
    aphel_p_d16,
    aphel_p_d32,
    aphel_p_d64,
    aphel_p_utf8,
};
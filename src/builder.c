#include "builder.h"
#include "term.h"
#include "arena.h"

u64 trace_size(luna_file* restrict f) {
    u64 size = 0;
    u64 cursor = 0;
    FOR_URANGE(i, 0, f->elems.len) {
        if (f->elems.at[i]->kind == ek_instruction) {
            switch (f->elems.at[i]->instr.code) {
            case aphel_d8:
                if (f->elems.at[i]->instr.args.len == 2) {
                    cursor += f->elems.at[i]->instr.args.at[1].as_literal;
                } else {
                    cursor += 1;
                }
                break;
            case aphel_d16:
                if (cursor % 2 != 0)
                    error_at_elem(f, f->elems.at[i], "d16 must be aligned to 2 bytes (use 'align 2' before)");
                if (f->elems.at[i]->instr.args.len == 2) {
                    cursor += 2 * f->elems.at[i]->instr.args.at[1].as_literal;
                } else {
                    cursor += 2;
                }
                break;
            case aphel_d32:
                if (cursor % 4 != 0)
                    error_at_elem(f, f->elems.at[i], "d32 must be aligned to 4 bytes (use 'align 4' before)");
                if (f->elems.at[i]->instr.args.len == 2) {
                    cursor += 4 * f->elems.at[i]->instr.args.at[1].as_literal;
                } else {
                    cursor += 4;
                }
                break;
            case aphel_d64:
                if (cursor % 8 != 0)
                    error_at_elem(f, f->elems.at[i], "d64 must be aligned to 8 bytes (use 'align 8' before)");
                if (f->elems.at[i]->instr.args.len == 2) {
                    cursor += 8 * f->elems.at[i]->instr.args.at[1].as_literal;
                } else {
                    cursor += 8;
                }
                break;
            case aphel_loc:
                cursor = f->elems.at[i]->instr.args.at[0].as_literal;
                break;
            case aphel_align:
                if (!is_pow_2(f->elems.at[i]->instr.args.at[0].as_literal))
                    error_at_elem(f, f->elems.at[i], "alignment must be a power of two");
                cursor = align_forward(cursor, f->elems.at[i]->instr.args.at[0].as_literal);
                break;
            case aphel_utf8:
                cursor += f->elems.at[i]->instr.args.at[0].as_str.len;
                break;
            default:
                if (cursor % 4 != 0)
                    error_at_elem(f, f->elems.at[i], "instructions must be aligned to 4 bytes (use 'align 4' before)");
                cursor += 4;
                break;
            }
            size = max(size, cursor);
        } else if (f->elems.at[i]->kind == ek_label) {
            f->elems.at[i]->label.symbol->value = cursor;
        } else {
            error_at_elem(f, f->elems.at[i], "what? (something went wrong, ask sandwichman about this)");
        }
    }
    return size;
}

u64 pure value(argument* restrict a) {
    if (a->kind == ak_literal) {
        return a->as_literal;
    } else if (a->kind == ak_symbol) {
        return a->as_symbol->value >> a->bit_shift_right;
    } else 
        general_error("crash: something very bad has happened internally");
}

// lmao
static forceinline void error_if_cant_sext(luna_file* restrict f, element* restrict e, i64 value, u8 bitwidth) {
    if (!can_losslessly_signext(value, bitwidth)) {
        warning_at_elem(f, e, "%lld is outside representable range [%lld, %lld]", value, sign_extend(1ull<<(bitwidth-1), bitwidth), (1ull<<(bitwidth-1))-1);
    }
}

static forceinline void error_if_cant_zext(luna_file* restrict f, element* restrict e, i64 value, u8 bitwidth) {
    if (!can_losslessly_zeroext(value, bitwidth)) {
        warning_at_elem(f, e, "%llu is outside representable range [0, %llu]", value, (1ull<<(bitwidth))-1);
    }
}

static forceinline void error_if_cant_sext_or_zext(luna_file* restrict f, element* restrict e, i64 value, u8 bitwidth) {
    if (!can_losslessly_zeroext(value, bitwidth) && !can_losslessly_signext(value, bitwidth)) {
        warning_at_elem(f, e, "%lld is outside representable ranges [%lld, %llu] ", value, sign_extend(1ull<<(bitwidth-1), bitwidth), (1ull<<(bitwidth))-1);
    }
}

void emit_binary(luna_file* restrict f, void* bin) {

    u64 cursor = 0;
    FOR_URANGE(i, 0, f->elems.len) {
        if (f->elems.at[i]->kind != ek_instruction) continue;
        // printf(" - %d\n", f->elems.at[i]->instr.code);
        switch (f->elems.at[i]->instr.code) {
        case aphel_d8: {
            u8 val;
            if (f->elems.at[i]->instr.args.at[0].kind == ak_symbol)
                val = f->elems.at[i]->instr.args.at[0].as_symbol->value;
            else
                val = f->elems.at[i]->instr.args.at[0].as_literal;

            error_if_cant_sext_or_zext(f, f->elems.at[i], val, 8);

            if (f->elems.at[i]->instr.args.len == 2) {
                u64 len = f->elems.at[i]->instr.args.at[1].as_literal;
                FOR_URANGE(i, 0, len) {
                    *((u8*)(cursor + (u64)bin)) = val;
                    cursor += 1;
                }
            } else {
                *((u8*)(cursor + (u64)bin)) = val;
                cursor += 1;
            }
            } break;
        case aphel_d16: {
            u16 val;
            if (f->elems.at[i]->instr.args.at[0].kind == ak_symbol)
                val = f->elems.at[i]->instr.args.at[0].as_symbol->value;
            else
                val = f->elems.at[i]->instr.args.at[0].as_literal;

            error_if_cant_sext_or_zext(f, f->elems.at[i], val, 16);
            if (f->elems.at[i]->instr.args.len == 2) {
                u64 len = f->elems.at[i]->instr.args.at[1].as_literal;
                FOR_URANGE(i, 0, len) {
                    *((u16*)(cursor + (u64)bin)) = val;
                    cursor += 2;
                }
            } else {
                *((u16*)(cursor + (u64)bin)) = val;
                cursor += 2;
            }
            } break;
        case aphel_d32: {
            u32 val;
            if (f->elems.at[i]->instr.args.at[0].kind == ak_symbol)
                val = f->elems.at[i]->instr.args.at[0].as_symbol->value;
            else
                val = f->elems.at[i]->instr.args.at[0].as_literal;

            error_if_cant_sext_or_zext(f, f->elems.at[i], val, 32);
            if (f->elems.at[i]->instr.args.len == 2) {
                u64 len = f->elems.at[i]->instr.args.at[1].as_literal;
                FOR_URANGE(i, 0, len) {
                    *((u32*)(cursor + (u64)bin)) = val;
                    cursor += 4;
                }
            } else {
                *((u32*)(cursor + (u64)bin)) = val;
                cursor += 4;
            }
            } break;
        case aphel_d64: {
            u64 val;
            if (f->elems.at[i]->instr.args.at[0].kind == ak_symbol)
                val = f->elems.at[i]->instr.args.at[0].as_symbol->value;
            else
                val = f->elems.at[i]->instr.args.at[0].as_literal;

            error_if_cant_sext_or_zext(f, f->elems.at[i], val, 64);
            if (f->elems.at[i]->instr.args.len == 2) {
                u64 len = f->elems.at[i]->instr.args.at[1].as_literal;
                FOR_URANGE(i, 0, len) {
                    *((u64*)(cursor + (u64)bin)) = val;
                    cursor += 8;
                }
            } else {
                *((u64*)(cursor + (u64)bin)) = val;
                cursor += 8;
            }
            } break;
        case aphel_loc: {
            u64 new_cursor = f->elems.at[i]->instr.args.at[0].as_literal;
            if (f->elems.at[i]->instr.args.len == 2 && cursor < new_cursor) {
                memset(((u8*)(cursor + (u64)bin)), value(&f->elems.at[i]->instr.args.at[1]), new_cursor - cursor);
            }
            cursor = new_cursor;
            } break;
        case aphel_align: {
            u64 new_cursor = align_forward(cursor, f->elems.at[i]->instr.args.at[0].as_literal);
            if (f->elems.at[i]->instr.args.len != 1) {
                memset(((u8*)(cursor + (u64)bin)), f->elems.at[i]->instr.args.at[1].as_literal, new_cursor - cursor);
            }
            cursor = new_cursor;
            } break;
        case aphel_utf8: {
            u64 len = f->elems.at[i]->instr.args.at[0].as_str.len;
            FOR_URANGE(j, 0, len) {
                *((u8*)(cursor + (u64)bin)) = f->elems.at[i]->instr.args.at[0].as_str.raw[j];
                cursor += 1;
            }
            } break;
        default:
            *(aphel_instruction*)(cursor + (u64)bin) = encode_instruction(f, f->elems.at[i], cursor);
            if (((aphel_instruction*)(cursor + (u64)bin))->opcode == 0) {
                error_at_elem(f, f->elems.at[i], "huh? %d/%d", i, f->elems.len);
            }
            cursor += 4;
            break;
        }
    }
}

aphel_instruction encode_instruction(luna_file* restrict f, element* restrict e, u64 position) {

    u8 branch_code;
    u8 li_code;
    u8 precision;

    aphel_reg rn = reg_rz;
    u16 off = 0;
    u8 sh = 0;

    switch (e->instr.code) {
    case aphel_int: error_if_cant_zext(f, e, value(&e->instr.args.at[0]), 8);
        return (aphel_instruction){
        .F.opcode = 0x01, .F.func = 0x00,
        .F.imm    = value(&e->instr.args.at[0])};
    case aphel_iret: return (aphel_instruction){
        .F.opcode = 0x01, .F.func = 0x01};
    case aphel_ires: return (aphel_instruction){
        .F.opcode = 0x01, .F.func = 0x02};
    case aphel_usr: return (aphel_instruction){
        .F.opcode = 0x01, .F.func = 0x03,
        .F.rde = e->instr.args.at[0].as_reg};
    case aphel_outr: return (aphel_instruction){
        .M.opcode = 0x02,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.rs1 = e->instr.args.at[1].as_reg};
    case aphel_outi: error_if_cant_zext(f, e, value(&e->instr.args.at[0]), 16); 
        return (aphel_instruction){
        .M.opcode = 0x03,
        .M.imm = value(&e->instr.args.at[0]),
        .M.rs1 = e->instr.args.at[1].as_reg};
    case aphel_inr: return (aphel_instruction){
        .M.opcode = 0x04,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.rs1 = e->instr.args.at[1].as_reg};
    case aphel_ini: error_if_cant_zext(f, e, value(&e->instr.args.at[1]), 16); 
        return (aphel_instruction){
        .M.opcode = 0x05,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.imm = value(&e->instr.args.at[1])};
    case aphel_jal: error_if_cant_sext(f, e, value(&e->instr.args.at[1]), 16); 
        return (aphel_instruction){
        .M.opcode = 0x06,
        .M.rs1 = e->instr.args.at[0].as_reg,
        .M.imm = value(&e->instr.args.at[1])};
    case aphel_jalr: error_if_cant_sext(f, e, value(&e->instr.args.at[1]), 16); 
        return (aphel_instruction){
        .M.opcode = 0x07,
        .M.rs1 = e->instr.args.at[0].as_reg,
        .M.imm = value(&e->instr.args.at[1]),
        .M.rde = e->instr.args.at[2].as_reg};
    case aphel_ret: return (aphel_instruction){
        .M.opcode = 0x08};
    case aphel_retr: return (aphel_instruction){
        .M.opcode = 0x09,
        .M.rs1 = e->instr.args.at[0].as_reg};
    case aphel_bra:  branch_code = 0x0; goto branch_encode;
    case aphel_beq:  branch_code = 0x1; goto branch_encode;
    case aphel_bez:  branch_code = 0x2; goto branch_encode;
    case aphel_blt:  branch_code = 0x3; goto branch_encode;
    case aphel_ble:  branch_code = 0x4; goto branch_encode;
    case aphel_bltu: branch_code = 0x5; goto branch_encode;
    case aphel_bleu: branch_code = 0x6; goto branch_encode;
    case aphel_bne:  branch_code = 0x9; goto branch_encode;
    case aphel_bnz:  branch_code = 0xA; goto branch_encode;
    case aphel_bge:  branch_code = 0xB; goto branch_encode;
    case aphel_bgt:  branch_code = 0xC; goto branch_encode;
    case aphel_bgeu: branch_code = 0xD; goto branch_encode;
    case aphel_bgtu: branch_code = 0xE;
        branch_encode:
        {
            i64 imm;
            if (e->instr.args.at[0].kind == ak_symbol) {
                u64 branch_val = e->instr.args.at[0].as_symbol->value >> e->instr.args.at[0].bit_shift_right;
                if (branch_val % 4 != 0) {
                    error_at_elem(f, e, "branch destination is not aligned to 4 bytes");
                }
                imm = (branch_val - position - 4);
            } else {
                imm = e->instr.args.at[0].as_literal * 4;
            }
            if (!can_losslessly_signext(imm, 20)) {
                error_at_elem(f, e, "branch is out of range");
            }

            return (aphel_instruction){
                .B.opcode = 0x0a,
                .B.func = branch_code,
                .B.imm = imm / 4};
        }
    case aphel_push: return (aphel_instruction){
        .M.opcode = 0x0b,
        .M.rs1 = e->instr.args.at[0].as_reg};
    case aphel_pop: return (aphel_instruction){
        .M.opcode = 0x0c,
        .M.rde = e->instr.args.at[0].as_reg};
    case aphel_enter: return (aphel_instruction){
        .B.opcode = 0x0d};
    case aphel_leave: return (aphel_instruction){
        .B.opcode = 0x0e};
    case aphel_lli:   li_code = 0; goto li_encode;
    case aphel_llis:  li_code = 1; goto li_encode;
    case aphel_lui:   li_code = 2; goto li_encode;
    case aphel_luis:  li_code = 3; goto li_encode;
    case aphel_lti:   li_code = 4; goto li_encode;
    case aphel_ltis:  li_code = 5; goto li_encode;
    case aphel_ltui:  li_code = 6; goto li_encode;
    case aphel_ltuis: li_code = 7;
        li_encode:
        error_if_cant_zext(f, e, value(&e->instr.args.at[1]), 16);
        return (aphel_instruction){
            .F.opcode = 0x10,
            .F.rde = e->instr.args.at[0].as_reg,
            .F.func = li_code,
            .F.imm = value(&e->instr.args.at[1])};
    case aphel_lw:  li_code = 0x11; goto loadmem_encode;
    case aphel_lh:  li_code = 0x12; goto loadmem_encode;
    case aphel_lhs: li_code = 0x13; goto loadmem_encode;
    case aphel_lq:  li_code = 0x14; goto loadmem_encode;
    case aphel_lqs: li_code = 0x15; goto loadmem_encode;
    case aphel_lb:  li_code = 0x16; goto loadmem_encode;
    case aphel_lbs: li_code = 0x17;
        loadmem_encode:
        switch (e->instr.args.len) {
        case 5:
            sh = value(&e->instr.args.at[4]);
            error_if_cant_zext(f, e, sh, 4);
        case 4:
            rn = e->instr.args.at[3].as_reg;
        case 3:
            off = value(&e->instr.args.at[2]);
            error_if_cant_sext(f, e, off, 8);
        }
        return (aphel_instruction){
            .E.opcode = li_code,
            .E.rde = e->instr.args.at[0].as_reg,
            .E.rs1 = e->instr.args.at[1].as_reg,
            .E.rs2 = rn,
            .E.imm = off,
            .E.func = sh
        };
    case aphel_sw: li_code = 0x18; goto storemem_encode;
    case aphel_sh: li_code = 0x19; goto storemem_encode;
    case aphel_sq: li_code = 0x1a; goto storemem_encode;
    case aphel_sb: li_code = 0x1b;

        storemem_encode:
        switch (e->instr.args.len) {
        case 5:
            sh = value(&e->instr.args.at[3]);
            error_if_cant_zext(f, e, sh, 4);
        case 4:
            rn = e->instr.args.at[2].as_reg;
        case 3:
            off = value(&e->instr.args.at[1]);
            error_if_cant_sext(f, e, off, 8);
        }
        return (aphel_instruction){
            .E.opcode = li_code,
            .E.rde = e->instr.args.at[e->instr.args.len-1].as_reg,
            .E.rs1 = e->instr.args.at[0].as_reg,
            .E.rs2 = rn,
            .E.imm = off,
            .E.func = sh
        };
    case aphel_cmpr: return (aphel_instruction){
        .M.opcode = 0x1e,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.rs1 = e->instr.args.at[1].as_reg};
    case aphel_cmpi: 
        if (e->instr.special == special_cmpi_reverse) {
            error_if_cant_sext(f, e, value(&e->instr.args.at[0]), 16);
            return (aphel_instruction){
                .F.opcode = 0x1f,
                .F.rde = e->instr.args.at[1].as_reg,
                .F.func = 1,
                .F.imm = value(&e->instr.args.at[0])};
        } else {
            error_if_cant_sext(f, e, value(&e->instr.args.at[1]), 16);
            return (aphel_instruction){
                .F.opcode = 0x1f,
                .F.rde = e->instr.args.at[0].as_reg,
                .F.func = 0,
                .F.imm = value(&e->instr.args.at[1])};
        }
    case aphel_addr: return (aphel_instruction){
        .R.opcode = 0x20,
        .R.rde = e->instr.args.at[0].as_reg,
        .R.rs1 = e->instr.args.at[1].as_reg,
        .R.rs2 = e->instr.args.at[2].as_reg};
    case aphel_addi: error_if_cant_sext(f, e, value(&e->instr.args.at[2]), 16);
        return (aphel_instruction){
        .M.opcode = 0x21,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.rs1 = e->instr.args.at[1].as_reg,
        .M.imm = value(&e->instr.args.at[2])};
    case aphel_subr: return (aphel_instruction){
        .R.opcode = 0x22,
        .R.rde = e->instr.args.at[0].as_reg,
        .R.rs1 = e->instr.args.at[1].as_reg,
        .R.rs2 = e->instr.args.at[2].as_reg};
    case aphel_subi: error_if_cant_sext(f, e, value(&e->instr.args.at[2]), 16);
        return (aphel_instruction){
        .M.opcode = 0x23,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.rs1 = e->instr.args.at[1].as_reg,
        .M.imm = value(&e->instr.args.at[2])};
    case aphel_imulr: return (aphel_instruction){
        .R.opcode = 0x24,
        .R.rde = e->instr.args.at[0].as_reg,
        .R.rs1 = e->instr.args.at[1].as_reg,
        .R.rs2 = e->instr.args.at[2].as_reg};
    case aphel_imuli: error_if_cant_sext(f, e, value(&e->instr.args.at[2]), 16);
        return (aphel_instruction){
        .M.opcode = 0x25,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.rs1 = e->instr.args.at[1].as_reg,
        .M.imm = value(&e->instr.args.at[2])};
    case aphel_idivr: return (aphel_instruction){
        .R.opcode = 0x26,
        .R.rde = e->instr.args.at[0].as_reg,
        .R.rs1 = e->instr.args.at[1].as_reg,
        .R.rs2 = e->instr.args.at[2].as_reg};
    case aphel_idivi: error_if_cant_sext(f, e, value(&e->instr.args.at[2]), 16);
        return (aphel_instruction){
        .M.opcode = 0x27,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.rs1 = e->instr.args.at[1].as_reg,
        .M.imm = value(&e->instr.args.at[2])};
    case aphel_umulr: return (aphel_instruction){
        .R.opcode = 0x28,
        .R.rde = e->instr.args.at[0].as_reg,
        .R.rs1 = e->instr.args.at[1].as_reg,
        .R.rs2 = e->instr.args.at[2].as_reg};
    case aphel_umuli: error_if_cant_zext(f, e, value(&e->instr.args.at[2]), 16);
        return (aphel_instruction){
        .M.opcode = 0x29,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.rs1 = e->instr.args.at[1].as_reg,
        .M.imm = value(&e->instr.args.at[2])};
    case aphel_udivr: return (aphel_instruction){
        .R.opcode = 0x2a,
        .R.rde = e->instr.args.at[0].as_reg,
        .R.rs1 = e->instr.args.at[1].as_reg,
        .R.rs2 = e->instr.args.at[2].as_reg};
    case aphel_udivi: error_if_cant_zext(f, e, value(&e->instr.args.at[2]), 16);
        return (aphel_instruction){
        .M.opcode = 0x2b,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.rs1 = e->instr.args.at[1].as_reg,
        .M.imm = value(&e->instr.args.at[2])};
    case aphel_remr: return (aphel_instruction){
        .R.opcode = 0x2c,
        .R.rde = e->instr.args.at[0].as_reg,
        .R.rs1 = e->instr.args.at[1].as_reg,
        .R.rs2 = e->instr.args.at[2].as_reg};
    case aphel_remi: error_if_cant_sext(f, e, value(&e->instr.args.at[2]), 16);
        return (aphel_instruction){
        .M.opcode = 0x2d,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.rs1 = e->instr.args.at[1].as_reg,
        .M.imm = value(&e->instr.args.at[2])};
    case aphel_modr: return (aphel_instruction){
        .R.opcode = 0x2e,
        .R.rde = e->instr.args.at[0].as_reg,
        .R.rs1 = e->instr.args.at[1].as_reg,
        .R.rs2 = e->instr.args.at[2].as_reg};
    case aphel_modi: error_if_cant_sext(f, e, value(&e->instr.args.at[2]), 16);
        return (aphel_instruction){
        .M.opcode = 0x2f,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.rs1 = e->instr.args.at[1].as_reg,
        .M.imm = value(&e->instr.args.at[2])};

    case aphel_andr: return (aphel_instruction){
        .R.opcode = 0x30,
        .R.rde = e->instr.args.at[0].as_reg,
        .R.rs1 = e->instr.args.at[1].as_reg,
        .R.rs2 = e->instr.args.at[2].as_reg};
    case aphel_andi: error_if_cant_zext(f, e, value(&e->instr.args.at[2]), 16);
        return (aphel_instruction){
        .M.opcode = 0x31,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.rs1 = e->instr.args.at[1].as_reg,
        .M.imm = value(&e->instr.args.at[2])};
    case aphel_orr: return (aphel_instruction){
        .R.opcode = 0x32,
        .R.rde = e->instr.args.at[0].as_reg,
        .R.rs1 = e->instr.args.at[1].as_reg,
        .R.rs2 = e->instr.args.at[2].as_reg};
    case aphel_ori: error_if_cant_zext(f, e, value(&e->instr.args.at[2]), 16);
        return (aphel_instruction){
        .M.opcode = 0x33,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.rs1 = e->instr.args.at[1].as_reg,
        .M.imm = value(&e->instr.args.at[2])};
    case aphel_norr: return (aphel_instruction){
        .R.opcode = 0x34,
        .R.rde = e->instr.args.at[0].as_reg,
        .R.rs1 = e->instr.args.at[1].as_reg,
        .R.rs2 = e->instr.args.at[2].as_reg};
    case aphel_nori: error_if_cant_zext(f, e, value(&e->instr.args.at[2]), 16);
        return (aphel_instruction){
        .M.opcode = 0x35,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.rs1 = e->instr.args.at[1].as_reg,
        .M.imm = value(&e->instr.args.at[2])};
    case aphel_xorr: return (aphel_instruction){
        .R.opcode = 0x36,
        .R.rde = e->instr.args.at[0].as_reg,
        .R.rs1 = e->instr.args.at[1].as_reg,
        .R.rs2 = e->instr.args.at[2].as_reg};
    case aphel_xori: error_if_cant_zext(f, e, value(&e->instr.args.at[2]), 16);
        return (aphel_instruction){
        .M.opcode = 0x37,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.rs1 = e->instr.args.at[1].as_reg,
        .M.imm = value(&e->instr.args.at[2])};
    case aphel_shlr: return (aphel_instruction){
        .R.opcode = 0x38,
        .R.rde = e->instr.args.at[0].as_reg,
        .R.rs1 = e->instr.args.at[1].as_reg,
        .R.rs2 = e->instr.args.at[2].as_reg};
    case aphel_shli: error_if_cant_zext(f, e, value(&e->instr.args.at[2]), 16);
        return (aphel_instruction){
        .M.opcode = 0x39,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.rs1 = e->instr.args.at[1].as_reg,
        .M.imm = value(&e->instr.args.at[2])};
    case aphel_asrr: return (aphel_instruction){
        .R.opcode = 0x3a,
        .R.rde = e->instr.args.at[0].as_reg,
        .R.rs1 = e->instr.args.at[1].as_reg,
        .R.rs2 = e->instr.args.at[2].as_reg};
    case aphel_asri: error_if_cant_zext(f, e, value(&e->instr.args.at[2]), 16);
        return (aphel_instruction){
        .M.opcode = 0x3b,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.rs1 = e->instr.args.at[1].as_reg,
        .M.imm = value(&e->instr.args.at[2])};
    case aphel_lsrr: return (aphel_instruction){
        .R.opcode = 0x3c,
        .R.rde = e->instr.args.at[0].as_reg,
        .R.rs1 = e->instr.args.at[1].as_reg,
        .R.rs2 = e->instr.args.at[2].as_reg};
    case aphel_lsri: error_if_cant_zext(f, e, value(&e->instr.args.at[2]), 16);
        return (aphel_instruction){
        .M.opcode = 0x3d,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.rs1 = e->instr.args.at[1].as_reg,
        .M.imm = value(&e->instr.args.at[2])};
    case aphel_bitr: return (aphel_instruction){
        .R.opcode = 0x3e,
        .R.rde = e->instr.args.at[0].as_reg,
        .R.rs1 = e->instr.args.at[1].as_reg,
        .R.rs2 = e->instr.args.at[2].as_reg};
    case aphel_biti: error_if_cant_zext(f, e, value(&e->instr.args.at[2]), 16);
        return (aphel_instruction){
        .M.opcode = 0x3f,
        .M.rde = e->instr.args.at[0].as_reg,
        .M.rs1 = e->instr.args.at[1].as_reg,
        .M.imm = value(&e->instr.args.at[2])};

    case aphel_fcmp16:
    case aphel_fcmp32:
    case aphel_fcmp64:
        precision = e->instr.code - aphel_fcmp16; return (aphel_instruction){
        .E.opcode = 0x40, .E.func = precision,
        .E.rde = e->instr.args.at[0].as_reg,
        .E.rs1 = e->instr.args.at[1].as_reg};
    case aphel_fto16:
    case aphel_fto32:
    case aphel_fto64:
        precision = e->instr.code - aphel_fto16; return (aphel_instruction){
        .E.opcode = 0x41, .E.func = precision,
        .E.rde = e->instr.args.at[0].as_reg,
        .E.rs1 = e->instr.args.at[1].as_reg};
    case aphel_ffrom16:
    case aphel_ffrom32:
    case aphel_ffrom64:
        precision = e->instr.code - aphel_ffrom16; return (aphel_instruction){
        .E.opcode = 0x42, .E.func = precision,
        .E.rde = e->instr.args.at[0].as_reg,
        .E.rs1 = e->instr.args.at[1].as_reg};
    case aphel_fneg16:
    case aphel_fneg32:
    case aphel_fneg64:
        precision = e->instr.code - aphel_fneg16; return (aphel_instruction){
        .E.opcode = 0x43, .E.func = precision,
        .E.rde = e->instr.args.at[0].as_reg,
        .E.rs1 = e->instr.args.at[1].as_reg};
    case aphel_fabs16:
    case aphel_fabs32:
    case aphel_fabs64:
        precision = e->instr.code - aphel_fabs16; return (aphel_instruction){
        .E.opcode = 0x44, .E.func = precision,
        .E.rde = e->instr.args.at[0].as_reg,
        .E.rs1 = e->instr.args.at[1].as_reg};
    case aphel_fadd16:
    case aphel_fadd32:
    case aphel_fadd64:
        precision = e->instr.code - aphel_fadd16; return (aphel_instruction){
        .E.opcode = 0x45, .E.func = precision,
        .E.rde = e->instr.args.at[0].as_reg,
        .E.rs1 = e->instr.args.at[1].as_reg,
        .E.rs2 = e->instr.args.at[2].as_reg};
    case aphel_fsub16:
    case aphel_fsub32:
    case aphel_fsub64:
        precision = e->instr.code - aphel_fsub16; return (aphel_instruction){
        .E.opcode = 0x46, .E.func = precision,
        .E.rde = e->instr.args.at[0].as_reg,
        .E.rs1 = e->instr.args.at[1].as_reg,
        .E.rs2 = e->instr.args.at[2].as_reg};
    case aphel_fmul16:
    case aphel_fmul32:
    case aphel_fmul64:
        precision = e->instr.code - aphel_fmul16; return (aphel_instruction){
        .E.opcode = 0x47, .E.func = precision,
        .E.rde = e->instr.args.at[0].as_reg,
        .E.rs1 = e->instr.args.at[1].as_reg,
        .E.rs2 = e->instr.args.at[2].as_reg};
    case aphel_fdiv16:
    case aphel_fdiv32:
    case aphel_fdiv64:
        precision = e->instr.code - aphel_fdiv16; return (aphel_instruction){
        .E.opcode = 0x48, .E.func = precision,
        .E.rde = e->instr.args.at[0].as_reg,
        .E.rs1 = e->instr.args.at[1].as_reg,
        .E.rs2 = e->instr.args.at[2].as_reg};
    case aphel_fma16:
    case aphel_fma32:
    case aphel_fma64:
        precision = e->instr.code - aphel_fma16; return (aphel_instruction){
        .E.opcode = 0x49, .E.func = precision,
        .E.rde = e->instr.args.at[0].as_reg,
        .E.rs1 = e->instr.args.at[1].as_reg,
        .E.rs2 = e->instr.args.at[2].as_reg};
    case aphel_fsqrt16:
    case aphel_fsqrt32:
    case aphel_fsqrt64:
        precision = e->instr.code - aphel_fsqrt16; return (aphel_instruction){
        .E.opcode = 0x4a, .E.func = precision,
        .E.rde = e->instr.args.at[0].as_reg,
        .E.rs1 = e->instr.args.at[1].as_reg};
    case aphel_fmin16:
    case aphel_fmin32:
    case aphel_fmin64:
        precision = e->instr.code - aphel_fmin16; return (aphel_instruction){
        .E.opcode = 0x4b, .E.func = precision,
        .E.rde = e->instr.args.at[0].as_reg,
        .E.rs1 = e->instr.args.at[1].as_reg,
        .E.rs2 = e->instr.args.at[2].as_reg};
    case aphel_fmax16:
    case aphel_fmax32:
    case aphel_fmax64:
        precision = e->instr.code - aphel_fmax16; return (aphel_instruction){
        .E.opcode = 0x4c, .E.func = precision,
        .E.rde = e->instr.args.at[0].as_reg,
        .E.rs1 = e->instr.args.at[1].as_reg,
        .E.rs2 = e->instr.args.at[2].as_reg};
    case aphel_fsat16:
    case aphel_fsat32:
    case aphel_fsat64:
        precision = e->instr.code - aphel_fsat16; return (aphel_instruction){
        .E.opcode = 0x4d, .E.func = precision,
        .E.rde = e->instr.args.at[0].as_reg,
        .E.rs1 = e->instr.args.at[1].as_reg};
    case aphel_fcnv16_32: precision = 0b0100; goto fcnv_encode;
    case aphel_fcnv16_64: precision = 0b1000; goto fcnv_encode;
    case aphel_fcnv32_16: precision = 0b0001; goto fcnv_encode;
    case aphel_fcnv32_64: precision = 0b1001; goto fcnv_encode;
    case aphel_fcnv64_16: precision = 0b0010; goto fcnv_encode;
    case aphel_fcnv64_32: precision = 0b0110;

        fcnv_encode:

        return (aphel_instruction){
            .E.opcode = 0x43, .E.func = precision,
            .E.rde = e->instr.args.at[0].as_reg,
            .E.rs1 = e->instr.args.at[1].as_reg};
    case aphel_fnan16:
    case aphel_fnan32:
    case aphel_fnan64:
        precision = e->instr.code - aphel_fnan16; return (aphel_instruction){
        .E.opcode = 0x4f, .E.func = precision,
        .E.rde = e->instr.args.at[0].as_reg,
        .E.rs1 = e->instr.args.at[1].as_reg};

    default:
        break;
    }
    return (aphel_instruction){};
}
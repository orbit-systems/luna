#include "builder.h"
#include "term.h"
#include "arena.h"

#define error_at_token(p, token, message, ...) \
    error_at_string((p)->path, (p)->text, (token).text, \
    message __VA_OPT__(,) __VA_ARGS__)

#define str_from_tokens(start, end) ((string){(start).text.raw, (end).text.raw - (start).text.raw + (end).text.len})

#define error_at_elem(f, elem, message, ...) do { \
    if (elem->kind == ek_instruction) { \
        u64 offset = elem->instr.args.len; \
        if (offset > 0) offset = offset * 2 - 1; \
        error_at_string(f->path, f->text, \
            str_from_tokens(f->tokens.at[elem->loc.start], f->tokens.at[elem->loc.start + offset]), \
            message __VA_OPT__(,) __VA_ARGS__); \
    } \
} while (0);

// assuming the element is of type ek_instruction
bool check_args(element* restrict e, arg_kind args[], size_t arglen) {
    if (e->instr.args.len != arglen) return false;
    FOR_URANGE(i, 0, arglen) {
        if (e->instr.args.at[i].kind != args[i]) return false;
    }
    return true;
}

// deals with macros and aliases
void check_and_expand(luna_file* restrict f) {

#   define current_elem (f->elems.at[i])
    FOR_URANGE(i, 0, f->elems.len) {
        if (current_elem->kind != ek_instruction) continue;
        switch (current_elem->instr.code) {
        case aphel_p_loc:
            if      (check_args(current_elem, (arg_kind[]){ak_literal, ak_literal}, 2)) continue;
            else if (check_args(current_elem, (arg_kind[]){ak_literal}, 1))             continue;
            else error_at_elem(f, current_elem, "invalid arguments for 'loc'");
            continue;
        case aphel_p_align:
            if      (check_args(current_elem, (arg_kind[]){ak_literal, ak_literal}, 2)) {
            }
            else if (check_args(current_elem, (arg_kind[]){ak_literal}, 1)) continue;
            else error_at_elem(f, current_elem, "invalid arguments for 'align'");
            continue;
        case aphel_p_nop:
            if (check_args(current_elem, NULL, 0)) {
                current_elem->instr.code = aphel_addr;
                da_append(&current_elem->instr.args, ((argument){.kind = ak_register, .as_reg = reg_rz}));
                da_append(&current_elem->instr.args, ((argument){.kind = ak_register, .as_reg = reg_rz}));
                da_append(&current_elem->instr.args, ((argument){.kind = ak_register, .as_reg = reg_rz}));
            } else error_at_elem(f, current_elem, "invalid arguments for 'nop'");
            continue;
        case aphel_p_inv:
            if (check_args(current_elem, NULL, 0)) {
                current_elem->instr.code = aphel_int;
                da_append(&current_elem->instr.args, ((argument){.kind = ak_literal, .as_literal = 2}));
            } else error_at_elem(f, current_elem, "invalid arguments for 'inv'");
            continue;
        case aphel_p_in:
            if      (check_args(current_elem, (arg_kind[]){ak_register, ak_register}, 2)) current_elem->instr.code = aphel_inr;
            else if (check_args(current_elem, (arg_kind[]){ak_literal, ak_register}, 2))  current_elem->instr.code = aphel_ini;
            else if (check_args(current_elem, (arg_kind[]){ak_symbol, ak_register}, 2))   current_elem->instr.code = aphel_ini;
            else error_at_elem(f, current_elem, "invalid arguments for 'in'");
            continue;
        case aphel_p_out:
            if      (check_args(current_elem, (arg_kind[]){ak_register, ak_register}, 2)) current_elem->instr.code = aphel_outr;
            else if (check_args(current_elem, (arg_kind[]){ak_register, ak_literal}, 2))  current_elem->instr.code = aphel_outi;
            else if (check_args(current_elem, (arg_kind[]){ak_register, ak_symbol}, 2))   current_elem->instr.code = aphel_outi;
            else error_at_elem(f, current_elem, "invalid arguments for 'out'");
            continue;
        case aphel_p_call:
        case aphel_p_callr:
            TODO("call and callr in check_and_expand");
        case aphel_p_li:
            // optimize li expansion
            if (check_args(current_elem, (arg_kind[]){ak_register, ak_literal}, 2)) {

                element e = *current_elem;
                if (can_losslessly_signext(e.instr.args.at[1].as_literal, 16)) {
                    // replace with simple llis
                    current_elem->instr.code = aphel_llis;
                } else if (can_losslessly_signext(e.instr.args.at[1].as_literal, 32)) {
                    current_elem->kind = ek_substream;
                    da_init(&current_elem->substream.elems, 2);
                    TODO("");
                } else if (can_losslessly_signext(e.instr.args.at[1].as_literal, 48)) {
                    TODO("");
                }

            } else if (check_args(current_elem, (arg_kind[]){ak_register, ak_symbol}, 2)) {
                element e = *current_elem;
            } else error_at_elem(f, current_elem, "invalid arguments for 'li'");
            continue;
        default:
            error_at_elem(f, current_elem, "im dumb and havent implemented this yet");
        continue;
        }
    }
#   undef current_elem

}

u64 trace_size(luna_file* restrict f) {
    u64 size = 0;

#   define current_elem (f->elems.at[i])
    FOR_URANGE(i, 0, f->elems.len) {
        if (current_elem->kind == ek_label) {
            current_elem->label.symbol->value = size;
            continue;
        }
        
        if (current_elem->kind == ek_instruction) {
            // simple instruction
            if (current_elem->instr.code > aphel_REAL_MIN && current_elem->instr.code < aphel_REAL_MAX) {
                size += 4;
                continue;
            }
        }

        UNREACHABLE("either borked or unfinished");
    }
#   undef current_elem

    return size;
}
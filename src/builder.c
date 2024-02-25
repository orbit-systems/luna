#include "builder.h"
#include "term.h"
#include "arena.h"

u64 trace_size(luna_file* restrict f) {
    u64 size = 0;
    u64 cursor = 0;
    FOR_URANGE(i, 0, f->elems.len) {
        if (f->elems.at[i]->kind == ek_instruction) {
            if (f->elems.at[i]->instr.code == aphel_d8) {
                if (f->elems.at[i]->instr.args.len == 2) {
                    cursor += f->elems.at[i]->instr.args.at[1].as_literal;
                } else {
                    cursor += 1;
                }
            } else if (f->elems.at[i]->instr.code == aphel_d16) {
                if (cursor % 2 != 0)
                    error_at_elem(f, f->elems.at[i], "d16 must be aligned to 2 bytes (use 'align 2' before)");
                if (f->elems.at[i]->instr.args.len == 2) {
                    cursor += 2 * f->elems.at[i]->instr.args.at[1].as_literal;
                } else {
                    cursor += 2;
                }
            } else if (f->elems.at[i]->instr.code == aphel_d32) {
                if (cursor % 4 != 0)
                    error_at_elem(f, f->elems.at[i], "d32 must be aligned to 4 bytes (use 'align 4' before)");
                if (f->elems.at[i]->instr.args.len == 2) {
                    cursor += 4 * f->elems.at[i]->instr.args.at[1].as_literal;
                } else {
                    cursor += 4;
                }
            } else if (f->elems.at[i]->instr.code == aphel_d64) {
                if (cursor % 8 != 0)
                    error_at_elem(f, f->elems.at[i], "d64 must be aligned to 8 bytes (use 'align 8' before)");
                if (f->elems.at[i]->instr.args.len == 2) {
                    cursor += 8 * f->elems.at[i]->instr.args.at[1].as_literal;
                } else {
                    cursor += 8;
                }
            } else if (f->elems.at[i]->instr.code == aphel_loc) {
                cursor = f->elems.at[i]->instr.args.at[0].as_literal;
            } else if (f->elems.at[i]->instr.code == aphel_align) {
                cursor = align_forward(cursor, f->elems.at[i]->instr.args.at[0].as_literal);
            } else if (f->elems.at[i]->instr.code == aphel_utf8) {
                cursor += strlen(f->elems.at[i]->instr.args.at[0].as_str);
            } else {
                if (cursor % 4 != 0)
                    error_at_elem(f, f->elems.at[i], "instructions must be aligned to 4 bytes (use 'align 4' before)");
                cursor += 4;
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
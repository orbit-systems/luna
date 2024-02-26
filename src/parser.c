#include "parser.h"
#include "term.h"

#define current_token (f->tokens.at[f->current_tok])
#define peek_token(n) (f->tokens.at[f->current_tok+(n)])
#define advance_token f->current_tok++
#define advance_token_n(n) f->current_tok += (n)
#define error_at_token(p, token, message, ...) \
    error_at_string((p)->path, (p)->text, (token).text, \
    message __VA_OPT__(,) __VA_ARGS__)

#define str_from_tokens(start, end) ((string){(start).text.raw, (end).text.raw - (start).text.raw + (end).text.len})

#define current_eq(cstr) (string_eq(current_token.text, to_string(cstr)))

element* generate_li(luna_file* restrict f, element* e) {
    if (check_args(e, (arg_kind[]){ak_register, ak_symbol}, 2)) {
        aphel_reg rd = e->instr.args.at[0].as_reg;
        symbol* sym = e->instr.args.at[1].as_symbol;
        
        e->instr.code = aphel_lli;
        e->instr.args.at[1].bit_shift_right = 0;
        da_append(&f->elems, e);

        e = new_element(&f->elem_alloca, ek_instruction);
        e->instr.code = aphel_lui;
        da_init(&e->instr.args, 2);
        da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = rd}));
        da_append(&e->instr.args, ((argument){.kind = ak_symbol, .as_symbol = sym, .bit_shift_right = 16}));
        da_append(&f->elems, e);

        e = new_element(&f->elem_alloca, ek_instruction);
        e->instr.code = aphel_lti;
        da_init(&e->instr.args, 2);
        da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = rd}));
        da_append(&e->instr.args, ((argument){.kind = ak_symbol, .as_symbol = sym, .bit_shift_right = 32}));
        da_append(&f->elems, e);

        e = new_element(&f->elem_alloca, ek_instruction);
        e->instr.code = aphel_lui;
        da_init(&e->instr.args, 2);
        da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = rd}));
        da_append(&e->instr.args, ((argument){.kind = ak_symbol, .as_symbol = sym, .bit_shift_right = 48}));

    } else if (check_args(e, (arg_kind[]){ak_register, ak_literal}, 2)) {
        aphel_reg rd = e->instr.args.at[0].as_reg;
        u64 val = e->instr.args.at[1].as_literal;
        if (can_losslessly_signext(val, 16)) {
            e->instr.code = aphel_llis;
        } else if (can_losslessly_signext(val, 32)) {
            e->instr.code = aphel_luis;
            e->instr.args.at[1].as_literal = (val >> 16) & 0xFFFF;
            da_append(&f->elems, e);

            e = new_element(&f->elem_alloca, ek_instruction);
            e->instr.code = aphel_lli;
            da_init(&e->instr.args, 2);
            da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = rd}));
            da_append(&e->instr.args, ((argument){.kind = ak_literal, .as_literal = (val & 0xFFFF)}));
        } else if (can_losslessly_signext(val, 48)) {
            e->instr.code = aphel_ltis;
            e->instr.args.at[1].as_literal = (val >> 32) & 0xFFFF;
            da_append(&f->elems, e);

            e = new_element(&f->elem_alloca, ek_instruction);
            e->instr.code = aphel_lui;
            da_init(&e->instr.args, 2);
            da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = rd}));
            da_append(&e->instr.args, ((argument){.kind = ak_literal, .as_literal = ((val >> 16) & 0xFFFF)}));
            da_append(&f->elems, e);
        
            e = new_element(&f->elem_alloca, ek_instruction);
            e->instr.code = aphel_lli;
            da_init(&e->instr.args, 2);
            da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = rd}));
            da_append(&e->instr.args, ((argument){.kind = ak_literal, .as_literal = (val & 0xFFFF)}));
        } else {
            e->instr.code = aphel_ltui;
            e->instr.args.at[1].as_literal = (val >> 48) & 0xFFFF;
            da_append(&f->elems, e);

            e = new_element(&f->elem_alloca, ek_instruction);
            e->instr.code = aphel_lti;
            da_init(&e->instr.args, 2);
            da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = rd}));
            da_append(&e->instr.args, ((argument){.kind = ak_literal, .as_literal = ((val >> 32) & 0xFFFF)}));
            da_append(&f->elems, e);
        
            e = new_element(&f->elem_alloca, ek_instruction);
            e->instr.code = aphel_lui;
            da_init(&e->instr.args, 2);
            da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = rd}));
            da_append(&e->instr.args, ((argument){.kind = ak_literal, .as_literal = ((val >> 16) & 0xFFFF)}));
            da_append(&f->elems, e);

            e = new_element(&f->elem_alloca, ek_instruction);
            e->instr.code = aphel_lli;
            da_init(&e->instr.args, 2);
            da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = rd}));
            da_append(&e->instr.args, ((argument){.kind = ak_literal, .as_literal = (val & 0xFFFF)}));
        }
    } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
    return e;
}

void parse_file(luna_file* restrict f) {
    while (current_token.type != tt_EOF) {
        if (current_token.type == tt_newline) {
            advance_token;
            continue;
        }

        // weed out some directives
        if current_eq("define") {
            advance_token;

            if (current_token.type != tt_identifier)
                error_at_token(f, current_token, "expected symbol name");

            string symname = current_token.text;
            symbol* s = symbol_find_or_create(f, symname);
            if (symname.raw[0] == '.')
                error_at_token(f, current_token, "cannot define local symbols with 'define'");
            if (s->defined)
                error_at_token(f, current_token, "symbol already defined");
            
            advance_token;
            if (current_token.type != tt_comma)
                error_at_token(f, current_token, "expected comma");
            

            advance_token;
            if (current_token.type != tt_literal_int && 
                current_token.type != tt_literal_char && 
                current_token.type != tt_literal_float)
                error_at_token(f, current_token, "expected int literal or char literal");
            s->value = parse_regular_literal(f);
            s->defined = true;            

            advance_token;
            if (current_token.type != tt_newline)
                error_at_token(f, current_token, "expected new line");
            advance_token;
            continue;
        }
        // labels
        if (current_token.type == tt_identifier && peek_token(1).type == tt_colon) {
            element* e = new_element(&f->elem_alloca, ek_label);
            e->loc.start = f->current_tok;
            if (current_token.text.raw[0] != '.') {
                // non-local label
                symbol* s = symbol_find_or_create(f, current_token.text);
                if (s->defined)
                    error_at_token(f, current_token, "symbol already defined");
                s->defined = true;
                e->label.symbol = s;
                f->last_nonlocal = s;
            } else {
                // local label
                if (f->last_nonlocal == NULL)
                    error_at_token(f, current_token, "local label requires a previous non-local label");   
                string symname = current_token.text;
                expand_local_sym(&symname, f->last_nonlocal, &f->str_alloca);
                symbol* s = symbol_find_or_create(f, symname);
                if (s->defined)
                    error_at_token(f, current_token, "local label already defined");
                // printf("defined local '"str_fmt"'\n", str_arg(s->name));
                s->defined = true;
                e->label.symbol = s;
            }
            advance_token_n(2);
            e->loc.len = f->current_tok - e->loc.start;
            da_append(&f->elems, e);
            continue;
        }

        if (current_token.type == tt_identifier) {
            element* e = new_element(&f->elem_alloca, ek_instruction);
            e->instr.code = 0;
            e->loc.start = f->current_tok;

            // lmfao
#           define INSTR(name_, namestr_, opcode_, func_, format_) if current_eq(namestr_) e->instr.code = aphel_##name_;
                INSTRUCTION_LIST
#           undef INSTR

            if (e->instr.code == 0)
                error_at_token(f, current_token, "unknown instruction");

            advance_token;
            da_init(&e->instr.args, 3);
            while (current_token.type != tt_newline) {
                if (current_token.type >= tt_register_rz && current_token.type <= tt_register_st) {
                    da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = (current_token.type - tt_register_rz)}));
                } else 
                if (current_token.type == tt_literal_char || current_token.type == tt_literal_int || current_token.type == tt_literal_float) {
                    da_append(&e->instr.args, ((argument){.kind = ak_literal, .as_literal = parse_regular_literal(f)}));
                } else 
                if (current_token.type == tt_literal_string) {
                    da_append(&e->instr.args, ((argument){.kind = ak_str, .as_str = string_lit_value(f)}));
                } else 
                if (current_token.type == tt_identifier) {
                    string symname = current_token.text;
                    if (symname.raw[0] == '.') {
                        if (f->last_nonlocal == NULL)
                            error_at_token(f, current_token, "reference to a local symbol requires a previous non-local label definition");
                        expand_local_sym(&symname, f->last_nonlocal, &f->str_alloca);
                    }
                    symbol* sym = symbol_find_or_create(f, symname);
                    da_append(&e->instr.args, ((argument){.kind = ak_symbol, .as_symbol = sym}));
                }
                advance_token;
                
                if (current_token.type == tt_newline) {
                    advance_token;
                    break;
                }
                if (current_token.type == tt_comma) {
                    advance_token;
                    continue;
                }
                if (current_token.type == tt_EOF) {
                    continue;
                }
                error_at_token(f, current_token, "expected ',' or newline");
            }


            e->loc.len = f->current_tok - e->loc.start;
            // check instructions, decode/dealias macros
            switch (e->instr.code) {
            case aphel_nop:
                if (check_args(e, NULL, 0)) {
                    e->instr.code = aphel_add;
                    da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = reg_rz}));
                    da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = reg_rz}));
                    da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = reg_rz}));
                } else {
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                }
                break;
            case aphel_inv:
                if (check_args(e, NULL, 0)) {
                    e->instr.code = aphel_int;
                    da_append(&e->instr.args, ((argument){.kind = ak_literal, .as_literal = 2}));
                } else {
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                }
                break;
            case aphel_int:
                if (!check_args(e, (arg_kind[]){ak_lit_or_sym}, 1))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_iret:
                if (!check_args(e, NULL, 0))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_ires:
                if (!check_args(e, NULL, 0))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_usr:
                if (!check_args(e, (arg_kind[]){ak_register}, 1))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;

            case aphel_out:
                if (check_args(e, (arg_kind[]){ak_register, ak_register}, 2)) {
                    e->instr.code = aphel_outr;
                } else if (check_args(e, (arg_kind[]){ak_lit_or_sym, ak_register}, 2)) {
                    e->instr.code = aphel_outi;
                } else
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_in:
                if (check_args(e, (arg_kind[]){ak_register, ak_register}, 2)) {
                    e->instr.code = aphel_inr;
                } else if (check_args(e, (arg_kind[]){ak_register, ak_lit_or_sym}, 2)) {
                    e->instr.code = aphel_ini;
                } else
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_outr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register}, 2))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_outi:
                if (!check_args(e, (arg_kind[]){ak_lit_or_sym, ak_register}, 2))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_inr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register}, 2))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_ini:
                if (!check_args(e, (arg_kind[]){ak_register, ak_lit_or_sym}, 2))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;

            case aphel_call:
                if (!check_args(e, (arg_kind[]){ak_register, ak_lit_or_sym}, 2))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                
                aphel_reg rs = e->instr.args.at[0].as_reg;
                // cursed
                e = generate_li(f, e);
                da_append(&f->elems, e);

                e = new_element(&f->elem_alloca, ek_instruction);
                e->instr.code = aphel_jal;
                da_init(&e->instr.args, 2);
                da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = rs}));
                da_append(&e->instr.args, ((argument){.kind = ak_literal, .as_literal = 0}));
                break;
            case aphel_callr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_lit_or_sym, ak_register}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));

                rs = e->instr.args.at[0].as_reg;
                aphel_reg rd = e->instr.args.at[2].as_reg;
                da_pop(&e->instr.args);
                // cursed
                e = generate_li(f, e);
                da_append(&f->elems, e);

                e = new_element(&f->elem_alloca, ek_instruction);
                e->instr.code = aphel_jalr;
                da_init(&e->instr.args, 2);
                da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = rs}));
                da_append(&e->instr.args, ((argument){.kind = ak_literal, .as_literal = 0}));
                da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = rd}));
                break;
            case aphel_jal:
                if (!check_args(e, (arg_kind[]){ak_register, ak_lit_or_sym}, 2))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_jalr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_lit_or_sym, ak_register}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_ret:
                if (!check_args(e, NULL, 0))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_retr:
                if (!check_args(e, (arg_kind[]){ak_register}, 1))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_bra:
            case aphel_beq:
            case aphel_bez:
            case aphel_blt:
            case aphel_ble:
            case aphel_bltu:
            case aphel_bleu:
            case aphel_bne:
            case aphel_bnz:
            case aphel_bge:
            case aphel_bgt:
            case aphel_bgeu:
            case aphel_bgtu:
                if (!check_args(e, (arg_kind[]){ak_lit_or_sym}, 1))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;

            case aphel_push:
                if (!check_args(e, (arg_kind[]){ak_register}, 1))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_pop:
                if (!check_args(e, (arg_kind[]){ak_register}, 1))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_enter:
                if (!check_args(e, NULL, 0))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_leave:
                if (!check_args(e, NULL, 0))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;

            case aphel_mov:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register}, 2))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                e->instr.code = aphel_orr;
                da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = reg_rz}));
                break;
            case aphel_li:
                e = generate_li(f, e);
                break;
            case aphel_lli:
            case aphel_llis:
            case aphel_lui:
            case aphel_luis:
            case aphel_lti:
            case aphel_ltis:
            case aphel_ltui:
            case aphel_ltuis:
                if (!check_args(e, (arg_kind[]){ak_register, ak_lit_or_sym}, 2))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;

            case aphel_lw:
            case aphel_lh:
            case aphel_lhs:
            case aphel_lq:
            case aphel_lqs:
            case aphel_lb:
            case aphel_lbs:
                if (check_args(e, (arg_kind[]){ak_register, ak_register}, 2)) {
                } else if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3)) {
                } else if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym, ak_register, ak_lit_or_sym}, 4)) {
                } else if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym, ak_register, ak_lit_or_sym}, 5)) {
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            
            case aphel_sw:
            case aphel_sh:
            case aphel_sq:
            case aphel_sb:
                if (check_args(e, (arg_kind[]){ak_register, ak_register}, 2)) {
                } else if (check_args(e, (arg_kind[]){ak_register, ak_lit_or_sym, ak_register}, 3)) {
                } else if (check_args(e, (arg_kind[]){ak_register, ak_lit_or_sym, ak_register, ak_register}, 4)) {
                } else if (check_args(e, (arg_kind[]){ak_register, ak_lit_or_sym, ak_register, ak_lit_or_sym, ak_register}, 5)) {
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;

            case aphel_cmp:
                if (check_args(e, (arg_kind[]){ak_register, ak_register}, 2)) {
                    e->instr.code = aphel_cmpr;
                } else if (check_args(e, (arg_kind[]){ak_register, ak_lit_or_sym}, 2)) {
                    e->instr.code = aphel_cmpi;
                } else if (check_args(e, (arg_kind[]){ak_lit_or_sym, ak_register}, 2)) {
                    e->instr.code = aphel_cmpi;
                    e->instr.special = special_cmpi_reverse;
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_cmpr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register}, 2)) {
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                }
                break;
            case aphel_cmpi:
                if (check_args(e, (arg_kind[]){ak_register, ak_lit_or_sym}, 2)) {
                } else if (check_args(e, (arg_kind[]){ak_lit_or_sym, ak_register}, 2)) {
                    e->instr.special = special_cmpi_reverse;
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            
            case aphel_add:
                if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3)) {
                    e->instr.code = aphel_addr;
                } else if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3)) {
                    e->instr.code = aphel_addi;
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_sub:
                if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3)) {
                    e->instr.code = aphel_subr;
                } else if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3)) {
                    e->instr.code = aphel_subi;
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_imul:
                if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3)) {
                    e->instr.code = aphel_imulr;
                } else if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3)) {
                    e->instr.code = aphel_imuli;
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_umul:
                if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3)) {
                    e->instr.code = aphel_umulr;
                } else if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3)) {
                    e->instr.code = aphel_umuli;
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_idiv:
                if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3)) {
                    e->instr.code = aphel_idivr;
                } else if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3)) {
                    e->instr.code = aphel_idivi;
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_udiv:
                if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3)) {
                    e->instr.code = aphel_udivr;
                } else if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3)) {
                    e->instr.code = aphel_udivi;
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_rem:
                if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3)) {
                    e->instr.code = aphel_remr;
                } else if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3)) {
                    e->instr.code = aphel_remi;
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_mod:
                if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3)) {
                    e->instr.code = aphel_modr;
                } else if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3)) {
                    e->instr.code = aphel_modi;
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_addr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_addi:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_subr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_subi:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_imulr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_imuli:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_umulr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_umuli:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
             case aphel_idivr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_idivi:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_udivr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_udivi:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_remr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_remi:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_modr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_modi:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;

            case aphel_and:
                if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3)) {
                    e->instr.code = aphel_andr;
                } else if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3)) {
                    e->instr.code = aphel_andi;
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_or:
                if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3)) {
                    e->instr.code = aphel_orr;
                } else if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3)) {
                    e->instr.code = aphel_ori;
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_nor:
                if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3)) {
                    e->instr.code = aphel_norr;
                } else if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3)) {
                    e->instr.code = aphel_nori;
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_xor:
                if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3)) {
                    e->instr.code = aphel_xorr;
                } else if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3)) {
                    e->instr.code = aphel_xori;
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_shl:
                if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3)) {
                    e->instr.code = aphel_shlr;
                } else if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3)) {
                    e->instr.code = aphel_shli;
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_asr:
                if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3)) {
                    e->instr.code = aphel_asrr;
                } else if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3)) {
                    e->instr.code = aphel_asri;
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_lsr:
                if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3)) {
                    e->instr.code = aphel_lsrr;
                } else if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3)) {
                    e->instr.code = aphel_lsri;
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_bit:
                if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3)) {
                    e->instr.code = aphel_bitr;
                } else if (check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3)) {
                    e->instr.code = aphel_biti;
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_andr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_andi:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_orr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_ori:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_norr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_nori:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_xorr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_xori:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_shlr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_shli:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_asrr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_asri:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_lsrr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_lsri:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_bitr:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_biti:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_lit_or_sym}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); break;
            case aphel_not:
                if (check_args(e, (arg_kind[]){ak_register, ak_register}, 2)) {
                    e->instr.code = aphel_nor;
                    da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = reg_rz}));
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_setfs:
                if (check_args(e, (arg_kind[]){ak_register}, 1)) {
                    e->instr.code = aphel_biti;
                    da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = reg_st}));
                    da_append(&e->instr.args, ((argument){.kind = ak_literal,  .as_literal = 0}));
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_setfz:
                if (check_args(e, (arg_kind[]){ak_register}, 1)) {
                    e->instr.code = aphel_biti;
                    da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = reg_st}));
                    da_append(&e->instr.args, ((argument){.kind = ak_literal,  .as_literal = 1}));
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_setfcb:
                if (check_args(e, (arg_kind[]){ak_register}, 1)) {
                    e->instr.code = aphel_biti;
                    da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = reg_st}));
                    da_append(&e->instr.args, ((argument){.kind = ak_literal,  .as_literal = 2}));
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_setfcbu:
                if (check_args(e, (arg_kind[]){ak_register}, 1)) {
                    e->instr.code = aphel_biti;
                    da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = reg_st}));
                    da_append(&e->instr.args, ((argument){.kind = ak_literal,  .as_literal = 3}));
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_setfe:
                if (check_args(e, (arg_kind[]){ak_register}, 1)) {
                    e->instr.code = aphel_biti;
                    da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = reg_st}));
                    da_append(&e->instr.args, ((argument){.kind = ak_literal,  .as_literal = 4}));
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_setfl:
                if (check_args(e, (arg_kind[]){ak_register}, 1)) {
                    e->instr.code = aphel_biti;
                    da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = reg_st}));
                    da_append(&e->instr.args, ((argument){.kind = ak_literal,  .as_literal = 5}));
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_setflu:
                if (check_args(e, (arg_kind[]){ak_register}, 1)) {
                    e->instr.code = aphel_biti;
                    da_append(&e->instr.args, ((argument){.kind = ak_register, .as_reg = reg_st}));
                    da_append(&e->instr.args, ((argument){.kind = ak_literal,  .as_literal = 6}));
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;

            
            case aphel_fadd16:
            case aphel_fsub16:
            case aphel_fmul16:
            case aphel_fdiv16:
            case aphel_fmin16:
            case aphel_fmax16:
            case aphel_fma16:
            case aphel_fadd32:
            case aphel_fsub32:
            case aphel_fmul32:
            case aphel_fdiv32:
            case aphel_fmin32:
            case aphel_fmax32:
            case aphel_fma32:
            case aphel_fadd64:
            case aphel_fsub64:
            case aphel_fmul64:
            case aphel_fdiv64:
            case aphel_fmin64:
            case aphel_fmax64:
            case aphel_fma64:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register, ak_register}, 3))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); 
                break;
            case aphel_fneg16:
            case aphel_fabs16:
            case aphel_fsqrt16:
            case aphel_fsat16:
            case aphel_fcnv16_32:
            case aphel_fcnv16_64:
            case aphel_fnan16:
            case aphel_fneg32:
            case aphel_fabs32:
            case aphel_fsqrt32:
            case aphel_fsat32:
            case aphel_fcnv32_16:
            case aphel_fcnv32_64:
            case aphel_fnan32:
            case aphel_fneg64:
            case aphel_fabs64:
            case aphel_fsqrt64:
            case aphel_fsat64:
            case aphel_fcnv64_16:
            case aphel_fcnv64_32:
            case aphel_fnan64:
            case aphel_fcmp16:
            case aphel_fcmp32:
            case aphel_fcmp64:
            case aphel_fto16:
            case aphel_fto32:
            case aphel_fto64:
            case aphel_ffrom16:
            case aphel_ffrom32:
            case aphel_ffrom64:
                if (!check_args(e, (arg_kind[]){ak_register, ak_register}, 2))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text)); 
                break;

            case aphel_skip:
                e->instr.code = aphel_d8;
                da_insert_at(&e->instr.args, ((argument){.kind = ak_literal, .as_literal = 0}), 0);
                break;
            case aphel_byte:
                e->instr.code = aphel_d8;
                break;
            case aphel_loc:
            case aphel_align:
                if (check_args(e, (arg_kind[]){ak_literal}, 1)) {
                } else if (check_args(e, (arg_kind[]){ak_literal, ak_literal}, 2)) {
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_d8:
            case aphel_d16:
            case aphel_d32:
            case aphel_d64:
                if (check_args(e, (arg_kind[]){ak_lit_or_sym}, 1)) {
                } else if (check_args(e, (arg_kind[]){ak_lit_or_sym, ak_literal}, 2)) {
                } else error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            case aphel_utf8:
                if (!check_args(e, (arg_kind[]){ak_str}, 1))
                    error_at_elem(f, e, "invalid arguments for '"str_fmt"'", str_arg(f->tokens.at[e->loc.start].text));
                break;
            default:
                error_at_elem(f, e, "what? (shouldnt happen, ask sandwichman about this)");
            }


            da_append(&f->elems, e);
            continue;
        }

        error_at_token(f, current_token, "expected an instruction, label, or directive");
    }
}

void check_definitions(luna_file* restrict f) {
    FOR_URANGE(i, 0, f->elems.len) {
        if (f->elems.at[i]->kind != ek_instruction) continue;

        FOR_URANGE(a, 0, f->elems.at[i]->instr.args.len) {
            if (f->elems.at[i]->instr.args.at[a].kind != ak_symbol) continue;
            symbol* sym = f->elems.at[i]->instr.args.at[a].as_symbol;
            if (!(f->elems.at[i]->instr.args.at[a].as_symbol->defined)) {
                u64 offset = a + 1;
                if (offset > 0) offset = offset * 2 - 1;
                error_at_token(f,
                    f->tokens.at[f->elems.at[i]->loc.start + offset], 
                    "symbol '"str_fmt"' undefined", str_arg(f->elems.at[i]->instr.args.at[a].as_symbol->name)
                );
            }
        }
    }
}

element* new_element(arena* restrict alloca, element_kind kind) {
    element* e = arena_alloc(alloca, sizeof(element), alignof(element));
    *e = (element){.kind = kind};
    return e;
}

symbol* symbol_find(luna_file* restrict f, string name) {
    FOR_URANGE(i, 0, f->symtab.len) {
        if (string_eq(f->symtab.at[i]->name, name)) return f->symtab.at[i];
    }
    return NULL;
}

symbol* symbol_find_or_create(luna_file* restrict f, string name) {
    symbol* sym = symbol_find(f, name);
    if (sym == NULL) {
        sym = arena_alloc(&f->elem_alloca, sizeof(symbol), alignof(symbol));
        da_append(&f->symtab, sym);
        sym->name = name;
        sym->defined = false;
        sym->value = 0;
    }
    return sym;
}

// assuming the element is of type ek_instruction
bool check_args(element* restrict e, arg_kind args[], size_t arglen) {
    if (e->instr.args.len != arglen) return false;
    FOR_URANGE(i, 0, arglen) {
        if (args[i] == ak_lit_or_sym) {
            if (e->instr.args.at[i].kind == ak_literal) continue;
            if (e->instr.args.at[i].kind == ak_symbol) continue;
        }
        if (e->instr.args.at[i].kind != args[i]) return false;
    }
    return true;
}

void expand_local_sym(string* restrict sym, symbol* restrict last_nonlocal, arena* restrict alloca) {
    if (sym->raw[0] != '.') return;
    string newname;
    newname.raw = arena_alloc(alloca, sym->len + last_nonlocal->name.len, 1);
    newname.len = sym->len + last_nonlocal->name.len;
    FOR_RANGE(i, 0, last_nonlocal->name.len) newname.raw[i] = last_nonlocal->name.raw[i];
    FOR_RANGE(i, 0, sym->len) newname.raw[last_nonlocal->name.len + i] = sym->raw[i];
    *sym = newname;
}

int ascii_to_digit_val(luna_file* restrict f, char c, u8 base) {
    char val = (char)base;
    if (c >= '0' && c <= '9') val = (char)(c-'0');
    if (c >= 'a' && c <= 'f') val = (char)(c-'a' + 10);
    if (c >= 'A' && c <= 'F') val = (char)(c-'A' + 10);
    
    if (val >= base)
        error_at_token(f, current_token, "invalid base %d digit '%c'", base, c);
    return val;
}

i64 char_lit_value(luna_file* restrict f) {
    string t = current_token.text;

    if (t.raw[1] != '\\') { // trivial case
        if (t.len > 3) error_at_token(f, current_token, "char literal too long");
        return t.raw[2];
    }

    switch(t.raw[2]) {
    case '0':  return 0;
    case 'a':  return '\a';
    case 'b':  return '\b';
    case 'e':  return 27;
    case 'f':  return '\f';
    case 'n':  return '\n';
    case 'r':  return '\r';
    case 't':  return '\t';
    case 'v':  return '\v';
    case '\\': return '\\';
    case '\"': return '\"';
    case '\'': return '\'';
    case 'x':
        if (t.len > 6) error_at_token(f, current_token, "char literal too long");
        return ascii_to_digit_val(f, t.raw[3], 16) * 0x10 + ascii_to_digit_val(f, t.raw[4], 16);
    case 'u':
        if (t.len > 8) error_at_token(f, current_token, "char literal too long");
        return ascii_to_digit_val(f, t.raw[3], 16) * 0x1000 + ascii_to_digit_val(f, t.raw[4], 16) * 0x0100 + 
               ascii_to_digit_val(f, t.raw[5], 16) * 0x0010 + ascii_to_digit_val(f, t.raw[6], 16);
    case 'U':
        if (t.len > 12) error_at_token(f, current_token, "char literal too long");
        return ascii_to_digit_val(f, t.raw[3], 16) * 0x10000000 + ascii_to_digit_val(f, t.raw[4], 16) * 0x01000000 + 
               ascii_to_digit_val(f, t.raw[5], 16) * 0x00100000 + ascii_to_digit_val(f, t.raw[6], 16) * 0x00010000 +
               ascii_to_digit_val(f, t.raw[7], 16) * 0x00001000 + ascii_to_digit_val(f, t.raw[8], 16) * 0x00000100 + 
               ascii_to_digit_val(f, t.raw[9], 16) * 0x00000010 + ascii_to_digit_val(f, t.raw[10], 16);

    default:
        error_at_token(f, current_token, "invalid escape sequence '%s'", clone_to_cstring(substring_len(t, 1, t.len-2)));
    }
    return -1;
}

f64 float_lit_value(luna_file* restrict f) {
    string t = current_token.text;
    f64 val = 0;

    int digit_start = 0;
    if (t.raw[0] == '-') {
        digit_start = 1;
    }

    int decimal_index = 0;
    for (int i = digit_start; i < t.len; i++) {
        if (t.raw[i] == '_') continue;
        if (t.raw[i] == '.') {
            decimal_index = i;
            break;
        }
        val = val*10 + (f64)ascii_to_digit_val(f, t.raw[i], 10);
    }

    int e_index = -1;
    f64 factor = 0.1;
    for (int i = decimal_index+1; i < t.len-1; i++) {
        if (t.raw[i] == 'e') {
            e_index = i;
            break;
        }
        if (t.raw[i] == '_') continue;
        val = val + (f64)ascii_to_digit_val(f, t.raw[i], 10) * factor;
        factor *= 0.1;
    }

    int exp_val = 0;

    if (e_index == -1) return val;
    if (t.raw[e_index+1] == '-') e_index++;
    
    for (int i = e_index+1; i < t.len-1; i++) {
        if (t.raw[i] == '_') continue;
        exp_val = exp_val*10 + ascii_to_digit_val(f, t.raw[i], 10);
    }

    val *= pow(10.0, (t.raw[e_index] == '-' ? -exp_val : exp_val));

    return digit_start == 1 ? -val : val;
}

i64 int_lit_value(luna_file* restrict f) {
    string t = current_token.text;
    i64 val = 0;

    bool is_negative = false;
    int digit_start = 0;

    if (t.raw[0] == '-') {
        is_negative = true;
        digit_start = 1;
    }

    if (t.raw[digit_start] != '0') { // basic base-10 parse
        FOR_URANGE(i, digit_start, t.len) {
            if (t.raw[i] == '_') continue;
            val = val * 10 + ascii_to_digit_val(f, t.raw[i], 10);
        }
        return val * (is_negative ? -1 : 1);
    }
    
    if (t.len == 1) return 0; // simple '0'
    if (is_negative && t.len == 2) return 0; // simple '-0'

    u8 base = 10;
    switch (t.raw[digit_start+1]) {
    case 'x':
    case 'X':
        base = 16; break;
    case 'o':
    case 'O':
        base = 8; break;
    case 'b':
    case 'B':
        base = 2; break;
    case 'd':
    case 'D':
        break;
    default:
        error_at_token(f, current_token, "invalid base prefix '%c'", t.raw[digit_start+1]);
    }

    if (t.len < 3 + digit_start) error_at_token(f, current_token, "expected digit after '%c'", t.raw[digit_start+1]);

    FOR_URANGE(i, 2 + digit_start, t.len) {
        if (t.raw[i] == '_') continue;
        val = val * base + ascii_to_digit_val(f, t.raw[i], base);
    }
    return val * (is_negative ? -1 : 1);
}

string string_lit_value(luna_file* restrict f) {
    string t = current_token.text;
    string val = NULL_STR;
    size_t val_len = 0;

    // trace string, figure out how long it needs to be
    FOR_URANGE(i, 1, t.len-1) {
        if (t.raw[i] != '\\') {
            val_len++;
            continue;
        }
        i++;
        switch (t.raw[i]) {
        case 'x':
            i++;
            i++;
        case '0':
        case 'a':
        case 'b':
        case 'e':
        case 'f':
        case 'n':
        case 'r':
        case 't':
        case 'v':
        case '\\':
        case '\"':
        case '\'':
            val_len++;
            break;
        default:
            error_at_token(f, current_token, "invalid escape sequence '\\%c'", t.raw[i]);
            break;
        }
    }

    // allocate
    val.raw = arena_alloc(&f->str_alloca, val_len, 1);
    val.len = val_len;

    // fill in string with correct bytes
    u64 val_i = 0;
    FOR_URANGE(i, 1, t.len-1) {
        if (t.raw[i] != '\\') {
            val.raw[val_i] = t.raw[i];
            val_i++;
            continue;
        }
        i++;
        switch (t.raw[i]) {
        case '0': val.raw[val_i] = '\0';  break;
        case 'a': val.raw[val_i] = '\a';  break;
        case 'b': val.raw[val_i] = '\b';  break;
        case 'e': val.raw[val_i] = '\e';  break;
        case 'f': val.raw[val_i] = '\f';  break;
        case 'n': val.raw[val_i] = '\n';  break;
        case 'r': val.raw[val_i] = '\r';  break;
        case 't': val.raw[val_i] = '\t';  break;
        case 'v': val.raw[val_i] = '\v';  break;
        case '\\': val.raw[val_i] = '\\'; break;
        case '\"': val.raw[val_i] = '\"'; break;
        case '\'': val.raw[val_i] = '\''; break;
        case 'x':
            val.raw[val_i] = ascii_to_digit_val(f, t.raw[i+1], 16) * 0x10 + ascii_to_digit_val(f, t.raw[i+2], 16);
            i += 2;
            break;
        default:
            error_at_token(f, current_token, "invalid escape sequence '\\%c'", t.raw[i]);
            break;
        }
        val_i++;
    }
    return val;
}

i64 parse_regular_literal(luna_file* restrict f) {
    switch (current_token.type) {
    case tt_literal_int:   
        return int_lit_value(f);
    case tt_literal_float: {
        f64 float_lit = float_lit_value(f);
        if (current_token.text.raw[current_token.text.len-1] == 'h') {
            f16 val = (f16)float_lit;
            return (i64)(u64)*(u16*)&val;
        }
        if (current_token.text.raw[current_token.text.len-1] == 'f') {
            f32 val = (f32)float_lit;
            return (i64)(u64)*(u32*)&val;
        }
        if (current_token.text.raw[current_token.text.len-1] == 'd') {
            f64 val = (f64)float_lit;
            return (i64)*(u64*)&val;
        }

        error_at_token(f, current_token, 
            "float literal requires precision valid specifier ('h', 'f', or 'd')");

        } break;
    case tt_literal_char:
        return char_lit_value(f);
    }
    return 0;
}
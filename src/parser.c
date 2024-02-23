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

void parse_file(luna_file* restrict f) {
    // ingest
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
                             
                // printf("defined '"str_fmt"'\n", str_arg(s->name));
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
            da_append(&f->instrs, e);
            continue;
        }

        if (current_token.type == tt_identifier) {
            element* e = new_element(&f->elem_alloca, ek_instruction);
            e->instr.code = 0;
            e->loc.start = f->current_tok;

            // this is fucking crazy lmao
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
            da_append(&f->instrs, e);
            continue;
        }

        error_at_token(f, current_token, "expected an instruction, label, or directive");
    }
}

void check_definitions(luna_file* restrict f) {
    FOR_URANGE(i, 0, f->instrs.len) {
        if (f->instrs.at[i]->kind != ek_instruction) continue;

        FOR_URANGE(a, 0, f->instrs.at[i]->instr.args.len) {
            if (f->instrs.at[i]->instr.args.at[a].kind != ak_symbol) continue;
            symbol* sym = f->instrs.at[i]->instr.args.at[a].as_symbol;
            if (!(f->instrs.at[i]->instr.args.at[a].as_symbol->defined)) {
                error_at_string(f->path, f->text, 
                    str_from_tokens(f->tokens.at[f->instrs.at[i]->loc.start], f->tokens.at[f->instrs.at[i]->loc.start + f->instrs.at[i]->loc.len-2]),
                    "symbol '"str_fmt"' undefined", str_arg(f->instrs.at[i]->instr.args.at[a].as_symbol->name)
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

    int decimal_index = 0;
    for (int i = digit_start; i < t.len; i++) {
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
        val = val + (f64)ascii_to_digit_val(f, t.raw[i], 10) * factor;
        factor *= 0.1;
    }

    int exp_val = 0;

    if (e_index == -1) return val;
    if (t.raw[e_index+1] == '-') e_index++;
    
    for (int i = e_index+1; i < t.len-1; i++) {
        exp_val = exp_val*10 + ascii_to_digit_val(f, t.raw[i], 10);
    }

    val *= pow(10.0, (t.raw[e_index] == '-' ? -exp_val : exp_val));

    // printf("\n\n%lf\n\n", (is_negative ? -val : val));

    return val;
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
        val = val * 10 + ascii_to_digit_val(f, t.raw[i], base);
    }
    return val * (is_negative ? -1 : 1);
}

char* string_lit_value(luna_file* restrict f) {
    string t = current_token.text;
    char* val = NULL;
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
    val = arena_alloc(&f->str_alloca, val_len + 1, 1);
    val[val_len] = '\0';

    // fill in string with correct bytes
    u64 val_i = 0;
    FOR_URANGE(i, 1, t.len-1) {
        if (t.raw[i] != '\\') {
            val[val_i] = t.raw[i];
            val_i++;
            continue;
        }
        i++;
        switch (t.raw[i]) {
        case '0': val[val_i] = '\0'; break;
        case 'a': val[val_i] = '\a'; break;
        case 'b': val[val_i] = '\b'; break;
        case 'e': val[val_i] = '\e'; break;
        case 'f': val[val_i] = '\f'; break;
        case 'n': val[val_i] = '\n'; break;
        case 'r': val[val_i] = '\r'; break;
        case 't': val[val_i] = '\t'; break;
        case 'v': val[val_i] = '\v'; break;
        case '\\': val[val_i] = '\\'; break;
        case '\"': val[val_i] = '\"'; break;
        case '\'': val[val_i] = '\''; break;
        case 'x':
            val[val_i] = ascii_to_digit_val(f, t.raw[i+1], 16) * 0x10 + ascii_to_digit_val(f, t.raw[i+2], 16);
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
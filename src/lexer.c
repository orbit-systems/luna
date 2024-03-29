#include "orbit.h"
#include "term.h"
#include "lexer.h"

// pretty much wholesale copied from the mars repo

#define can_start_identifier(ch) ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_' || ch == '.')
#define can_be_in_identifier(ch) ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '_' || ch == '.')
#define can_start_number(ch) ((ch >= '0' && ch <= '9') || ch == '-')
#define valid_digit(ch) ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '_'))

#define valid_0x(ch) ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F') || (ch == '_'))
#define valid_0d(ch) ((ch >= '0' && ch <= '9') || (ch == '_'))
#define valid_0o(ch) ((ch >= '0' && ch <= '7') || (ch == '_'))
#define valid_0b(ch) (ch == '0' || ch == '1' || ch == '_')

#define current_char(lex) (lex->current_char)
#define advance_char(lex) (lex->cursor < lex->src.len ? (lex->current_char = lex->src.raw[++lex->cursor]) : '\0')
#define advance_char_n(lex, n) (lex->cursor + (n) < lex->src.len ? (lex->current_char = lex->src.raw[lex->cursor += (n)]) : '\0')
#define peek_char(lex, n) ((lex->cursor + (n)) < lex->src.len ? lex->src.raw[lex->cursor + (n)] : '\0')


int skip_block_comment(lexer* restrict lex);
void skip_until_char(lexer* restrict lex, char c);
void skip_whitespace(lexer* restrict lex);

token_type scan_ident_or_keyword(lexer* restrict lex);
token_type scan_number(lexer* restrict lex);
token_type scan_string_or_char(lexer* restrict lex);
token_type scan_operator(lexer* restrict lex);

string lex_string_lit_value(token* restrict tok);

char* token_type_str[] = {
#define TOKEN(enum, str) str,
    TOKEN_LIST
#undef TOKEN
};

file* find_file_from_slice(da(file)* restrict files, string slice) {
    FOR_URANGE(i, 0, files->len) {
        if (is_within(files->at[i].src, slice)) return &files->at[i];
    }
    return NULL;
}

lexer new_lexer(string path, string src, da(file) incl) {
    lexer lex = {0};
    lex.path = path;
    lex.src = src;
    lex.current_char = src.raw[0];
    lex.cursor = 0;
    lex.included = incl;
    if (incl.at == NULL) {
        da_init(&lex.included, 1);
    }
    da_append(&lex.included, ((file){path, src}));
    da_init(&lex.buffer, src.len/3.5);
    return lex;
}

bool has_been_included(lexer* restrict lex, string path) {
    FOR_URANGE(i, 0, lex->included.len) {
        if (string_eq(lex->included.at[i].path, path)) return true;
    }
    return false;
}

#define top_token(l) ((l)->buffer.at[(l)->buffer.len-1])

void construct_token_buffer(lexer* restrict lex) {
    if (lex == NULL || is_null_str(lex->src) || is_null_str(lex->path))
        CRASH("bad lexer provided to construct_token_buffer");

    do {
        append_next_token(lex);
        bool is_forced = string_eq(top_token(lex).text, to_string("forceinclude"));
        if (top_token(lex).type == tt_identifier && (string_eq(top_token(lex).text, to_string("include")) || is_forced)) {
            append_next_token(lex);
            string path = lex_string_lit_value(&top_token(lex));
            if (is_null_str(path)) error_at_string(lex->path, lex->src, top_token(lex).text, "invalid string literal");
            if (!fs_exists(path)) error_at_string(lex->path, lex->src, top_token(lex).text, "cannot find file");

            fs_file f;
            fs_get(path, &f);
            if (!fs_is_regular(&f)) error_at_string(lex->path, lex->src, top_token(lex).text, "file at path is not a regular file");

            string src = string_alloc(f.size);

            bool can_open = fs_open(&f, "rb");
            if ((has_been_included(lex, path)) && !is_forced) {
                warning_at_string(lex->path, lex->src, top_token(lex).text, "file already included, skipped");
                da_pop(&lex->buffer); // destroy 'include'
                da_pop(&lex->buffer); // destroy path
            } else {
                fs_read_entire(&f, src.raw);

                // da_append(&lex->included, ((file){f.path, src}));

                lexer f_lex = new_lexer(f.path, src, lex->included);

                // hijack token buffer
                da_pop(&lex->buffer); // destroy 'include'
                da_pop(&lex->buffer); // destroy path

                da_destroy(&f_lex.buffer);
                f_lex.buffer = lex->buffer;
                construct_token_buffer(&f_lex);

                // patch token buffer back in
                if (top_token(&f_lex).type == tt_EOF) da_pop(&f_lex.buffer);
                lex->buffer = f_lex.buffer;
                lex->included = f_lex.included;

                fs_close(&f);
            }

            
            // fs_drop(&f);
        }

    } while (top_token(lex).type != tt_EOF);

    da_shrink(&lex->buffer);
}

void append_next_token(lexer* restrict lex) {

    // if the top token is an EOF, early return
    // if (lex->buffer.at[lex->buffer.len-1].type == tt_EOF) {
    //     return;
    // }
    

    // advance to next significant char
    while (true) {
        switch (current_char(lex)) {
        case ' ':
        case '\t':
        case '\r':
        case '\v':
            advance_char(lex);
            break;
        case ';':
            skip_until_char(lex, '\n');
            break;
        case '/':
            if (peek_char(lex, 1) == '/') {
                skip_until_char(lex, '\n');
            } else if (peek_char(lex, 1) == '*') {
                advance_char_n(lex, 2);
                int final_level = skip_block_comment(lex);
                if (final_level != 0) {
                    error_at_string(lex->path, lex->src, string_make(&lex->src.raw[lex->cursor], 1), 
                        "unclosed block comment"
                    );
                }
            } else goto skip_insignificant_end;
            break;
        default:
            goto skip_insignificant_end;
        }
    }
    skip_insignificant_end:

    if (lex->cursor >= lex->src.len) {
        da_append(
            &lex->buffer, 
            ((token){substring_len(lex->src, lex->cursor, 1), tt_newline})
        );
        da_append(
            &lex->buffer, 
            ((token){substring_len(lex->src, lex->cursor, 1), tt_EOF})
        );
        return;
    }

    u64 beginning_cursor = lex->cursor;
    token_type this_type;
    if (can_start_identifier(current_char(lex))) {
        this_type = scan_ident_or_keyword(lex);
    } else if (current_char(lex) == '\n') {
        this_type = tt_newline;
        advance_char(lex);
    } else if (can_start_number(current_char(lex))) {
        this_type = scan_number(lex);
    } else if (current_char(lex) == '\"' || current_char(lex) == '\'') {
        this_type = scan_string_or_char(lex);
    } else {
        this_type = scan_operator(lex);
    }

    da_append(&lex->buffer, ((token){
        .text = substring(lex->src, beginning_cursor, lex->cursor), 
        .type = this_type,
    }));
}

token_type scan_ident_or_keyword(lexer* restrict lex) {
    u64 beginning = lex->cursor;
    
    advance_char(lex);
    while (can_be_in_identifier(current_char(lex))) 
        advance_char(lex);

    string word = substring(lex->src, beginning, lex->cursor);

    if (string_eq(word, to_string("."))) return tt_period;

    if (string_eq(word, to_string("rz"))) return tt_register_rz;
    if (string_eq(word, to_string("ra"))) return tt_register_ra;
    if (string_eq(word, to_string("rb"))) return tt_register_rb;
    if (string_eq(word, to_string("rc"))) return tt_register_rc;
    if (string_eq(word, to_string("rd"))) return tt_register_rd;
    if (string_eq(word, to_string("re"))) return tt_register_re;
    if (string_eq(word, to_string("rf"))) return tt_register_rf;
    if (string_eq(word, to_string("rg"))) return tt_register_rg;
    if (string_eq(word, to_string("rh"))) return tt_register_rh;
    if (string_eq(word, to_string("ri"))) return tt_register_ri;
    if (string_eq(word, to_string("rj"))) return tt_register_rj;
    if (string_eq(word, to_string("rk"))) return tt_register_rk;
    if (string_eq(word, to_string("ip"))) return tt_register_ip;
    if (string_eq(word, to_string("sp"))) return tt_register_sp;
    if (string_eq(word, to_string("fp"))) return tt_register_fp;
    if (string_eq(word, to_string("st"))) return tt_register_st;

    return tt_identifier;
}

token_type scan_number(lexer* restrict lex) {
    advance_char(lex);
    while (true) {
        if (current_char(lex) == '.') {
            
            // really quick, check if its one of the range operators? this causes bugs very often :sob:
            if (peek_char(lex, 1) == '.') {
                return tt_literal_int;
            }

            advance_char(lex);
            while (true) {
                if (current_char(lex) == 'e' && peek_char(lex, 1) == '-') {
                    advance_char_n(lex, 2);
                }
                if (!valid_digit(current_char(lex))) {
                    return tt_literal_float;
                }
                advance_char(lex);
            }
        }
        if (!valid_digit(current_char(lex))) {
            return tt_literal_int;
        }
        advance_char(lex);
    }
}
token_type scan_string_or_char(lexer* restrict lex) {
    char quote_char = current_char(lex);
    u64  start_cursor = lex->cursor;

    advance_char(lex);
    while (true) {
        if (current_char(lex) == '\\') {
            advance_char(lex);
        } else if (current_char(lex) == quote_char) {
            advance_char(lex);
            return quote_char == '\"' ? tt_literal_string : tt_literal_char;
        } else if (current_char(lex) == '\n') {
            if (quote_char == '\"') error_at_string(lex->path, lex->src, substring(lex->src, start_cursor, lex->cursor),
                "unclosed string literal");
            if (quote_char == '\'') error_at_string(lex->path, lex->src, substring(lex->src, start_cursor, lex->cursor),
                "unclosed char literal");
        }
        advance_char(lex);
    }
}
token_type scan_operator(lexer* restrict lex) {
    switch (current_char(lex)) {
    case '+':
        advance_char(lex);
        if (current_char(lex) == '=') {
            advance_char(lex);
            return tt_add_equal;
        }
        return tt_add;
    case '-':
        advance_char(lex);

        if (current_char(lex) == '=') {
            advance_char(lex);
            return tt_sub_equal;
        }
        if (current_char(lex) == '>') {
            advance_char(lex);
            return tt_arrow_right;
        }
        if (current_char(lex) == '-' && peek_char(lex, 1) == '-') {
            advance_char_n(lex, 2);
            return tt_uninit;
        }
        return tt_sub;
    case '*':
        advance_char(lex);
        if (current_char(lex) == '=') {
            advance_char(lex);
            return tt_mul_equal;
        }
        return tt_mul;
    case '/':
        advance_char(lex);
        if (current_char(lex) == '=') {
            advance_char(lex);
            return tt_div_equal;
        }
        return tt_div;
    case '%':
        advance_char(lex);
        if (current_char(lex) == '=') {
            advance_char(lex);
            return tt_mod_equal;
        }
        if (current_char(lex) == '%') {
            advance_char(lex);
                if (current_char(lex) == '=') {
                advance_char(lex);
                return tt_mod_mod_equal;
            }
            return tt_mod_mod;
        }
        return tt_mod;
    case '~':
        advance_char(lex);
        if (current_char(lex) == '=') {
            advance_char(lex);
            return tt_xor_equal;
        }
        if (current_char(lex) == '|') {
            advance_char(lex);
            if (current_char(lex) == '=') {
                advance_char(lex);
                return tt_nor_equal;
            }
            return tt_nor;
        }
        return tt_tilde;
    case '&':
        advance_char(lex);
        if (current_char(lex) == '=') {
            advance_char(lex);
            return tt_and_equal;
        }
        if (current_char(lex) == '&') {
            advance_char(lex);
            return tt_and_and;
        }
        return tt_and;
    case '|':
        advance_char(lex);
        if (current_char(lex) == '|') {
            advance_char(lex);
            return tt_or_or;
        }
        if (current_char(lex) == '=') {
            advance_char(lex);
            return tt_or_equal;
        }
        return tt_or;
    case '<':
        advance_char(lex);
        if (current_char(lex) == '<') {
            advance_char(lex);
            if (current_char(lex) == '=') {
                advance_char(lex);
                return tt_lshift_equal;
            }
            return tt_lshift;
        }
        if (current_char(lex) == '=') {
            advance_char(lex);
            return tt_less_equal;
        }
        return tt_less_than;
    case '>':
        advance_char(lex);
        if (current_char(lex) == '>') {
            advance_char(lex);
            if (current_char(lex) == '=') {
                advance_char(lex);
                return tt_rshift_equal;
            }
            return tt_rshift;
        }
        if (current_char(lex) == '=') {
            advance_char(lex);
            return tt_greater_equal;
        }
        return tt_greater_than;
    case '=':
        advance_char(lex);
        if (current_char(lex) == '=') {
            advance_char(lex);
            return tt_equal_equal;
        }
        return tt_equal;
    case '!':
        advance_char(lex);
        if (current_char(lex) == '=') {
            advance_char(lex);
            return tt_not_equal;
        }
        return ttam;
    case ':':
        advance_char(lex);
        if (current_char(lex) == ':') {
            advance_char(lex);
            return tt_colon_colon;
        }
        return tt_colon;

    case '#': advance_char(lex); return tt_hash;
    case ';': advance_char(lex); return tt_semicolon;
    case '$': advance_char(lex); return tt_dollar;
    case ',': advance_char(lex); return tt_comma;
    case '^': advance_char(lex); return tt_carat;
    case '@': advance_char(lex); return tt_at;

    case '(': advance_char(lex); return tt_open_paren;
    case ')': advance_char(lex); return tt_close_paren;
    case '[': advance_char(lex); return tt_open_bracket;
    case ']': advance_char(lex); return tt_close_bracket;
    case '{': advance_char(lex); return tt_open_brace;
    case '}': advance_char(lex); return tt_close_brace;

    default:
        error_at_string(lex->path, lex->src, substring_len(lex->src, lex->cursor, 1), 
            "unrecognized character");
        break;
    }
    return tt_invalid;
}

int skip_block_comment(lexer* restrict lex) {
    int level = 1;
    while (level != 0) {
        if (lex->cursor >= lex->src.len) {
            break;
        }
        if (current_char(lex) == '/' && peek_char(lex, 1) == '*') {
            advance_char_n(lex, 2);
            level++;
        }
        else if (current_char(lex) == '*' && peek_char(lex, 1) == '/') {
            advance_char_n(lex, 2);
            level--;
        } else
            advance_char(lex);
    }
    return level;
}

void skip_until_char(lexer* restrict lex, char c) {
    while (current_char(lex) != c && lex->cursor < lex->src.len) {
        advance_char(lex);
    }
}

void skip_whitespace(lexer* restrict lex) {
    while (true) {
        char r = current_char(lex);
        if ((r != ' ' && r != '\t' && r != '\r' && r != '\v') || lex->cursor >= lex->src.len) {
            return;
        }
        advance_char(lex);
    }
}


// shitty copy of string_lit_value to work in the lexer
string lex_string_lit_value(token* restrict tok) {
    string t = tok->text;
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
            return NULL_STR;
            break;
        }
    }

    // allocate
    val.raw = malloc(val_len);
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
        default:
            return NULL_STR;
            break;
        }
        val_i++;
    }
    return val;
}

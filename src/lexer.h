#pragma once
#define LEXER_H

#include "luna.h"
#include "dynarr.h"

typedef u8 token_type; enum {
    tt_invalid,

    tt_char_literal,    // 'a'
    tt_int_literal,     // 800, 0x800, 0o127, 0b230
    tt_string_literal,  // "real"
    
    tt_identifier,      // real_shit-093
    
    tt_newline,         // new line '\n'
    tt_open_bracket,    // [
    tt_close_bracket,   // ]
    tt_open_brace,      // {
    tt_close_brace,     // }
    tt_equal,           // =
    tt_comma,           // ,
    tt_period,          // .
    tt_colon,           // :

    tt_EOF,             // end of file
};

extern char* token_type_str[];

typedef struct token_s {
    u32        start;
    u16        len;
    token_type type;
} token;

dynarr_lib_h(token) // generate dynamic array library
#define dynarr_token_get(buf, index) (index < buf.len ? buf.base[index] : EOF_TOKEN)

typedef struct lexer_state_s {
    char*     text;
    u64       text_len;
    char*     text_path;
    u64       cursor;
    dynarr(token) tokens;
    char      current_char;
} lexer_state;

#define can_start_identifier(ch) ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_')
#define can_start_number(ch) ((ch >= '0' && ch <= '9') || ch == '-')
#define valid_digit(ch) ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))
#define valid_0x(ch) ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))
#define valid_0d(ch) (ch >= '0' && ch <= '9')
#define valid_0o(ch) (ch >= '0' && ch <= '7')
#define valid_0b(ch) (ch == '0' || ch == '1')

#define EOF_TOKEN ((token){0,0, tt_EOF})

#define current_char(lex) (lex->current_char)
#define advance_char(lex) (lex->cursor < lex->text_len ? (lex->current_char = lex->text[++lex->cursor]) : '\0')
#define advance_char_n(lex, n) (lex->cursor+n < lex->text_len ? (lex->current_char = lex->text[lex->cursor += n]) : '\0')
#define peek_char(lex, amnt) ((lex->cursor + amnt) < lex->text_len ? lex->text[lex->cursor + amnt] : '\0')
void append_next_token(lexer_state* l);

void print_token(lexer_state* l, token* t);

void lexer_init(lexer_state* lexer, char* text_path, char* text, u64 text_len);
void append_next_token(lexer_state* l);

void scan_identifier(lexer_state* l);
void scan_int_literal(lexer_state* l);
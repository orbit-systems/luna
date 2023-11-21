#pragma once
#define LUNA_LEXER_H

#include "luna.h"

typedef u8 token_type; enum {

    tt_char_literal,    // 'a'
    tt_int_literal,     // 800, 0x800, 0o127, 0b230
    tt_string_literal,  // "real"
    
    tt_identifier,      // real_shit
    
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

typedef struct token_s {
    u64 start;
    u64 len;
    token_type type;
} token;

typedef struct token_buf_s {
    token* base;
    u64    len;
    u64    cap;
} token_buf;

typedef struct lexer_state_s {
    char* text;
    u64   text_len;
    char* text_path;

    u64   cursor;

    token_buf tokens;

} lexer_state;

#define EOF_TOKEN (token){0,0, tt_EOF}

#define current_char(lex) (lex.cursor < lex.text_len ? lex.text[lex.cursor] : '\0')
#define advance_char(lex) (lex.cursor+1 < lex.text_len ? lex.text[lex.cursor] : '\0')
#define peek_char(lex, amnt) ((lex.cursor + amnt) < lex.text_len ? lex.text[lex.cursor + amnt] : '\0')

void tokenize(lexer_state* lexer);
void token_buf_init(token_buf* buffer, u64 capacity);
void token_buf_append(token_buf* buffer, token t);
#define token_buf_get(buf, index) (index < buf.len ? buf.base[index] : EOF_TOKEN)
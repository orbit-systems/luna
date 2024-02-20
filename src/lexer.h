#pragma once
#define LEXER_H

// pretty much wholesale copied from the mars repo

#include "orbit.h"

#define TOKEN_LIST \
    TOKEN(tt_invalid, "INVALID") \
    TOKEN(tt_EOF,     "EOF") \
\
    TOKEN(tt_newline,    "newline")\
    TOKEN(tt_identifier, "identifier") \
\
    TOKEN(tt_literal_int, "integer literal") \
    TOKEN(tt_literal_float, "float literal") \
    TOKEN(tt_literal_string, "string literal") \
    TOKEN(tt_literal_char, "char literal") \
\
    TOKEN(tt_hash,      "#") \
    TOKEN(tt_uninit,    "---") \
    TOKEN(tt_equal,     "=") \
    TOKEN(tt_dollar,    "$") \
    TOKEN(tt_colon,     ":") \
    TOKEN(tt_colon_colon, "::") \
    TOKEN(tt_semicolon, ";") \
    TOKEN(tt_comma,     ",") \
    TOKEN(tt_period,     ".") \
    TOKEN(ttam,         "!") \
    TOKEN(tt_carat,     "^") \
    TOKEN(tt_at,        "@") \
    TOKEN(tt_add,       "+") \
    TOKEN(tt_sub,       "-") \
    TOKEN(tt_mul,       "*") \
    TOKEN(tt_div,       "/") \
    TOKEN(tt_mod,       "%") \
    TOKEN(tt_mod_mod,   "%%") \
    TOKEN(tt_tilde,     "~") \
    TOKEN(tt_and,       "&") \
    TOKEN(tt_or,        "|") \
    TOKEN(tt_nor,       "~|") \
    TOKEN(tt_lshift,    "<<") \
    TOKEN(tt_rshift,    ">>") \
\
    TOKEN(tt_and_and,       "&&") \
    TOKEN(tt_or_or,         "||") \
    TOKEN(tt_tilde_tilde,   "~~") \
\
    TOKEN(tt_arrow_right,   "->") \
    TOKEN(tt_arrow_left,    "<-") \
    TOKEN(tt_arrow_bidir,   "<->") \
\
    TOKEN(tt_add_equal,     "+=") \
    TOKEN(tt_sub_equal,     "-=") \
    TOKEN(tt_mul_equal,     "*=") \
    TOKEN(tt_div_equal,     "/=") \
    TOKEN(tt_mod_equal,     "%=") \
    TOKEN(tt_mod_mod_equal, "%%=") \
\
    TOKEN(tt_and_equal,     "&=") \
    TOKEN(tt_or_equal,      "|=") \
    TOKEN(tt_nor_equal,     "~|=") \
    TOKEN(tt_xor_equal,     "~=") \
    TOKEN(tt_lshift_equal,  "<<=") \
    TOKEN(tt_rshift_equal,  ">>=") \
\
    TOKEN(tt_equal_equal,   "==") \
    TOKEN(tt_not_equal,     "!=") \
    TOKEN(tt_less_than,     "<") \
    TOKEN(tt_less_equal,    "<=") \
    TOKEN(tt_greater_than,  ">") \
    TOKEN(tt_greater_equal, ">=") \
\
    TOKEN(tt_open_paren,    "(") \
    TOKEN(tt_close_paren,   ")") \
    TOKEN(tt_open_brace,    "{") \
    TOKEN(tt_close_brace,   "}") \
    TOKEN(tt_open_bracket,  "[") \
    TOKEN(tt_close_bracket, "]") \
\
    TOKEN(tt_meta_COUNT, "") \

typedef u8 token_type; enum {
#define TOKEN(enum, str) enum,
TOKEN_LIST
#undef TOKEN
};

extern char* token_type_str[];

typedef struct token_s {
    string text;
    token_type type;
} token;

da_typedef(token);

typedef struct lexer_s {
    string src;
    string path;
    da(token) buffer;
    u64 cursor;
    char current_char;
} lexer;

lexer new_lexer(string path, string src);
void construct_token_buffer(lexer* lex);
void append_next_token(lexer* lex);
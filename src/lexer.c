#include "lexer.h"
#include "luna.h"

void token_buf_init(token_buf* buffer, u64 capacity) {
    if (capacity == 0) capacity = 1;

    *buffer = (token_buf){NULL, 0, capacity};
    buffer->base = (token*) malloc(sizeof(token) * capacity);
}

void token_buf_append(token_buf* buffer, token t) {
    if (buffer->len == buffer->cap) {
        buffer->cap *= 1.75;
        buffer->base = (token*) realloc(buffer->base, sizeof(token) * buffer->cap);
    }
    buffer->base[buffer->len++] = t;
}

void token_buf_shrink(token_buf* buffer) {
    buffer->base = (token*) realloc(buffer->base, sizeof(token) * buffer->len);
}

void lexer_init(lexer_state* lexer, char* text_path, char* text, u64 text_len) {
    *lexer = (lexer_state){};
    lexer->text = text;
    lexer->text_len = text_len;
    lexer->text_path = text_path;
    token_buf_init(&lexer->tokens, 16);
}

void next_token(lexer_state* l) {
    loop: while(true) {
        switch (current_char(l)){
        case '\t':
        case '\v':
        case ' ':
            advance_char(l);
            continue;

        case '\n': token_buf_append(l, (token){l->cursor, 1, tt_newline});       advance_char(l); return;
        case '[':  token_buf_append(l, (token){l->cursor, 1, tt_open_bracket});  advance_char(l); return;
        case ']':  token_buf_append(l, (token){l->cursor, 1, tt_close_bracket}); advance_char(l); return;
        case '{':  token_buf_append(l, (token){l->cursor, 1, tt_open_brace});    advance_char(l); return;
        case '}':  token_buf_append(l, (token){l->cursor, 1, tt_close_brace});   advance_char(l); return;
        case '=':  token_buf_append(l, (token){l->cursor, 1, tt_equal});         advance_char(l); return;
        case ',':  token_buf_append(l, (token){l->cursor, 1, tt_comma});         advance_char(l); return;
        case '.':  token_buf_append(l, (token){l->cursor, 1, tt_period});        advance_char(l); return;
        case ':':  token_buf_append(l, (token){l->cursor, 1, tt_colon});         advance_char(l); return;

        case '\0': token_buf_append(l, EOF_TOKEN); return;
        
        default: die("unrecognized char '%c'\n", current_char(l));
        break;
        }
    }
}
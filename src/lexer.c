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

void append_next_token(lexer_state* l) {
    loop: while(true) {
        switch (current_char(l)){
        case '\t':
        case '\v':
        case ' ':
            advance_char(l);
            continue;
        
        case '/':
            if (peek_char(l, 1) == '/') {
                while(current_char(l) != '\n') advance_char(l);
                continue;
            }
            if (peek_char(l, 1) == '*') {
                advance_char_n(l, 2);
                int level = 1;
                while (level != 0) {
                    if (current_char(l) == '\0') {
                        die("unclosed block comment");
                    }
                    if (current_char(l) == '/' && peek_char(l, 1) == '*') {
                        advance_char_n(l,2);
                        level++;
                    } else if (current_char(l) == '*' && peek_char(l, 1) == '/') {
                        advance_char_n(l,2);
                        level--;
                    }
                }
                continue;
            } else {
                die("expected '*' or '/' after '/'\n");
            }
        
        case '\'':
            {
            u64 beginning = l->cursor;
            advance_char(l);
            while (true) {
                switch (current_char(l)) {
                case '\'':
                    advance_char(l);
                    token_buf_append(&l->tokens, (token){beginning, l->cursor - beginning, tt_string_literal});
                    return;
                case '\\':
                    advance_char_n(l,2);
                    break;
                case '\n':
                    die("unclosed char literal");
                default:
                    advance_char(l);
                    break;
                }
            }}
            break;
        case '\"':
            {
            u64 beginning = l->cursor;
            advance_char(l);
            while (true) {
                switch (current_char(l)) {
                case '\"':
                    advance_char(l);
                    token_buf_append(&l->tokens, (token){beginning, l->cursor - beginning, tt_string_literal});
                    return;
                case '\\':
                    advance_char_n(l,2);
                    break;
                case '\n':
                    die("unclosed string literal");
                default:
                    advance_char(l);
                    break;
                }
            }}
            break;
        case '\n': token_buf_append(&l->tokens, (token){l->cursor, 1, tt_newline});       advance_char(l); return;
        case '[':  token_buf_append(&l->tokens, (token){l->cursor, 1, tt_open_bracket});  advance_char(l); return;
        case ']':  token_buf_append(&l->tokens, (token){l->cursor, 1, tt_close_bracket}); advance_char(l); return;
        case '{':  token_buf_append(&l->tokens, (token){l->cursor, 1, tt_open_brace});    advance_char(l); return;
        case '}':  token_buf_append(&l->tokens, (token){l->cursor, 1, tt_close_brace});   advance_char(l); return;
        case '=':  token_buf_append(&l->tokens, (token){l->cursor, 1, tt_equal});         advance_char(l); return;
        case ',':  token_buf_append(&l->tokens, (token){l->cursor, 1, tt_comma});         advance_char(l); return;
        case '.':  token_buf_append(&l->tokens, (token){l->cursor, 1, tt_period});        advance_char(l); return;
        case ':':  token_buf_append(&l->tokens, (token){l->cursor, 1, tt_colon});         advance_char(l); return;
        case '\0': token_buf_append(&l->tokens, EOF_TOKEN); return;
        default: 
            if (can_start_identifier(current_char(l))) {
                u64 start_index = l->cursor;
                scan_identifier(l);
                u16 length = l->cursor - start_index;
                token_buf_append(&l->tokens, (token){start_index, length, tt_identifier});
                return;
            } else if (can_start_number(current_char(l))) {
                u64 start_index = l->cursor;
                scan_int_literal(l);
                u16 length = l->cursor - start_index;
                token_buf_append(&l->tokens, (token){start_index, length, tt_int_literal});
                return;
            } else {
                die("unrecognized char '%c'\n", current_char(l));
            }
            break;
        }
    }
}

void scan_identifier(lexer_state* l) {
    advance_char(l);
    while (can_start_identifier(current_char(l)) || can_start_number(current_char(l)))
        advance_char(l);
}

void scan_int_literal(lexer_state* l) {
    if (current_char(l) == '-') {
        advance_char(l);
    }
    if (current_char(l) == '0') {
        switch (current_char(l)) {
        case 'x':
            while (valid_0x(current_char(l))) advance_char(l);
            break;
        case 'o':
            while (valid_0o(current_char(l))) advance_char(l);
            break;
        case 'b':
            while (valid_0b(current_char(l))) advance_char(l);
            break;
        case 'd':
            advance_char(l);
        default:
            while (valid_0d(current_char(l))) advance_char(l);
            break;
        }
    }
}
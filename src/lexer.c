#include "lexer.h"
#include "luna.h"

void tokenize(lexer_state* lexer) {

}

void token_buf_init(token_buf* buffer, u64 capacity) {
    if (capacity == 0) capacity = 1;

    *buffer = (token_buf){NULL, 0, capacity};
    buffer->base = (token*) malloc(sizeof(token) * capacity);
}

void token_buf_append(token_buf* buffer, token t) {
    if (buffer->len == buffer->cap) {
        buffer->cap *= 2;
        buffer->base = (token*) realloc(buffer->base, sizeof(token) * buffer->cap);
    }
    buffer->base[buffer->len++] = t;
}
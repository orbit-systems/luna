#include "luna.h"
#include "error.h"

void error_at_position( char* path, char* text, u32 position, u16 length, char* message, ...) {
    
    char ERROR_MSG_BUFFER[500] = {};
    va_list args;
    va_start(args, message);
    vsprintf(ERROR_MSG_BUFFER, message, args);
    va_end(args);

    set_style(STYLE_FG_Red);
    set_style(STYLE_Bold);
    printf("ERROR");
    set_style(STYLE_Reset);

    set_style(STYLE_Dim);
    printf(" |");
    set_style(STYLE_Reset);

    int line;
    int column;
    char* line_ptr;
    int line_len;
    line_and_col(text, position, &line_ptr, &line_len, &line, &column);

    printf(" %s @ %d:%d ", path, line, column);

    set_style(STYLE_Dim);
    printf("-> ");
    set_style(STYLE_Reset);

    set_style(STYLE_Italic);
    set_style(STYLE_Bold);
    printf("%s", ERROR_MSG_BUFFER);
    set_style(STYLE_Reset);

    set_style(STYLE_Dim);
    printf("\n      | ");
    printf("\n % 4d | ", line);
    set_style(STYLE_Reset);
    printn(line_ptr, line_len);
    set_style(STYLE_Dim);
    printf("\n      | ");
    set_style(STYLE_Reset);
    
    while (column-- > 1) printf(" ");


    set_style(STYLE_FG_Red);
    set_style(STYLE_Bold);
    if (length > 0) {
        printf("^");
    }
    if (length > 2) {
        for (int i = 2; i < length; i++)
            printf("~");
    }
    if (length > 1) {
        printf("^");
    }
    set_style(STYLE_Reset);

    set_style(STYLE_Italic);
    set_style(STYLE_Bold);
    printf(" %s\n", ERROR_MSG_BUFFER);
    set_style(STYLE_Reset);

    exit(EXIT_FAILURE);

}

void warning_at_position( char* path, char* text, u32 position, u16 length, char* message, ...) {
    
    char ERROR_MSG_BUFFER[500] = {};
    va_list args;
    va_start(args, message);
    vsprintf(ERROR_MSG_BUFFER, message, args);
    va_end(args);

    set_style(STYLE_FG_Yellow);
    set_style(STYLE_Bold);
    printf("WARNING");
    set_style(STYLE_Reset);

    set_style(STYLE_Dim);
    printf(" |");
    set_style(STYLE_Reset);

    int line;
    int column;
    char* line_ptr;
    int line_len;
    line_and_col(text, position, &line_ptr, &line_len, &line, &column);

    printf(" %s @ %d:%d ", path, line, column);

    set_style(STYLE_Dim);
    printf("-> ");
    set_style(STYLE_Reset);

    set_style(STYLE_Italic);
    set_style(STYLE_Bold);
    printf("%s", ERROR_MSG_BUFFER);
    set_style(STYLE_Reset);

    set_style(STYLE_Dim);
    printf("\n        | ");
    printf("\n   % 4d | ", line);
    set_style(STYLE_Reset);
    printn(line_ptr, line_len);
    set_style(STYLE_Dim);
    printf("\n        | ");
    set_style(STYLE_Reset);
    
    while (column-- > 1) printf(" ");

    set_style(STYLE_FG_Yellow);
    set_style(STYLE_Bold);
    if (length > 0) {
        printf("^");
    }
    if (length > 2) {
        for (int i = 2; i < length; i++)
            printf("~");
    }
    if (length > 1) {
        printf("^");
    }
    set_style(STYLE_Reset);

    set_style(STYLE_Italic);
    set_style(STYLE_Bold);
    printf(" %s\n", ERROR_MSG_BUFFER);
    set_style(STYLE_Reset);

}


void printn(char* text, size_t len) {
    size_t c = 0;
    while (text[c] != '\0' && c < len)
        putchar(text[c++]);
}

void line_and_col(char* text, size_t position, char** last_newline, int* line_len, int* line, int* col) {
    
    int l = 0;
    int c = 0;

    for (int i = 0; i <= position; i++) {
        if (text[i] == '\n') {
            *last_newline = &text[i];
            l++;
            c = 0;
            continue;
        }
        if (text[i] == '\t') {
            c += 4;
            continue;
        }
        c++;
    }

    *last_newline += 1;
    *line = l + 1;
    *col = c;

    *line_len = 0;
    while ((*last_newline)[*line_len] != '\0' && (*last_newline)[*line_len] != '\n') 
        *line_len += 1;
}
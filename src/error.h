#include "luna.h"

enum ANSI_STYLE {
    STYLE_Reset       = 0,

    STYLE_Bold        = 1,
    STYLE_Dim         = 2,
    STYLE_Italic      = 3,

    STYLE_FG_Black    = 30,
    STYLE_FG_Red      = 31,
    STYLE_FG_Green    = 32,
    STYLE_FG_Yellow   = 33,
    STYLE_FG_Blue     = 34,
    STYLE_FG_Magenta  = 35,
    STYLE_FG_Cyan     = 36,
    STYLE_FG_White    = 37,
    STYLE_FG_Default  = 39,

    STYLE_BG_Black    = 40,
    STYLE_BG_Red      = 41,
    STYLE_BG_Green    = 42,
    STYLE_BG_Yellow   = 43,
    STYLE_BG_Blue     = 44,
    STYLE_BG_Magenta  = 45,
    STYLE_BG_Cyan     = 46,
    STYLE_BG_White    = 47,
    STYLE_BG_Default  = 49,
};

#define set_style(style) printf("\x1b[%dm", style)

void error_at_position(char* path, char* text, u32 position, u16 length, char* message, ...);
void warning_at_position( char* path, char* text, u32 position, u16 length, char* message, ...);

void printn(char* text, size_t len);
void line_and_col(char* text, size_t position, char** last_newline, int* line_len, int* line, int* col);
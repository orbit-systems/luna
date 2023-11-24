#pragma once
#define CORE_H

#include "luna.h"

typedef struct cmd_arg_s {
    char* key;
    char* val;
} cmd_arg;

typedef struct flag_set_s {
    char* input_path;
    char* output_path;
} flag_set;

cmd_arg make_argument(char* s);
void load_arguments(int argc, char* argv[], flag_set* fl);

void print_help();
u8* load_file(FILE* asm_file);
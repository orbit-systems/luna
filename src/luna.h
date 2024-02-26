#pragma once
#define LUNA_H

#include "orbit.h"
#include "term.h"

typedef struct cmd_arg_s {
    string key;
    string val;
} cmd_arg;

typedef struct flag_set_s {
    string input_path;
    string output_path;
    bool print_timings;
    bool dump_symtab;
} flag_set;

cmd_arg make_argument(char* s);
void load_arguments(int argc, char* argv[], flag_set* fl);

void print_help();

extern flag_set luna_flags;
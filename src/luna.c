// ╭──────╮
// │ luna │ the Aphelion ISA assembler
// ╰──────╯
// by spsandwichman

// using aphelion v0.4

#define ORBIT_IMPLEMENTATION
#include "luna.h"
#include "lexer.h"
#include "parser.h"

flag_set luna_flags;

int main(int argc, char** argv) {
    load_arguments(argc, argv, &luna_flags);

    if (!fs_exists(luna_flags.input_path)) {
        general_error("cannot find file \""str_fmt"\"", str_arg(luna_flags.input_path));
    }

    bool status = true;
    fs_file input;
    status = status && fs_get(luna_flags.input_path, &input);
    string input_text = string_alloc(input.size);
    status = status && fs_open(&input, "rb");
    status = status && fs_read_entire(&input, input_text.raw);
    status = status && fs_close(&input);

    if (!status) {
        general_error("failed to read file \""str_fmt"\"", str_arg(luna_flags.input_path));
    }

    lexer input_lexer = new_lexer(luna_flags.input_path, input_text);
    construct_token_buffer(&input_lexer);

    luna_file source = {0};
    source.text = input_lexer.src;
    source.path = input_lexer.path;
    source.tokens = input_lexer.buffer;
    da_init(&source.symtab, 16);
    da_init(&source.instrs, 16);
    source.elem_alloca = arena_make(0x4000);
    source.str_alloca  = arena_make(0x1000);

    parse_file(&source);

    printf("SYMBOL TABLE\n");
    u64 max_sym_len = source.symtab.at[0].name.len;
    FOR_URANGE(i, 1, source.symtab.len) {
        max_sym_len = max(max_sym_len, source.symtab.at[i].name.len);
    }
    FOR_URANGE(i, 0, source.symtab.len) {
        printf("    "str_fmt, str_arg(source.symtab.at[i].name));

        FOR_URANGE(j, 0, max_sym_len - source.symtab.at[i].name.len + 4) putchar(' ');

        printf("%s  %16lx\n",
            source.symtab.at[i].defined ? "def  " : "undef", source.symtab.at[i].value
        );
    }
}


void print_help() {
    printf("usage: luna (source file) [flags]\n");
    printf("\n");
    printf("-o:(path)           specify an output path\n");
    printf("-help               display this text\n\n");
    printf("-timings            print assembler stage timings\n");
}

cmd_arg make_argument(char* s) {
    for (size_t i = 0; s[i] != '\0'; i++) {
        if (s[i] == ':') {
            s[i] = 0;
            return (cmd_arg){to_string(s), to_string(s+i+1)};
        }
    }
    return (cmd_arg){to_string(s), NULL_STR};
}

void load_arguments(int argc, char* argv[], flag_set* fl) {
    if (argc < 2) {
        print_help();
        exit(EXIT_SUCCESS);
    }

    *fl = (flag_set){0};

    cmd_arg input_directory_arg = make_argument(argv[1]);
    if (string_eq(input_directory_arg.key, to_string("-help"))) {
        print_help();
        exit(EXIT_SUCCESS);
    }
    if (!is_null_str(input_directory_arg.val)) {
        general_error("expected an input path, got \"%s\"", argv[1]);
    }
    fl->input_path = input_directory_arg.key;

    if (argc <= 2) return;

    int flag_start_index = 2;
    FOR_RANGE(i, flag_start_index, argc) {
        cmd_arg a = make_argument(argv[i]);
        if (string_eq(a.key, to_string("-help"))) {
            print_help();
            exit(EXIT_SUCCESS);
        } else if (string_eq(a.key, to_string("-o"))) {
            if (is_null_str(a.val) || a.val.raw[0] == '\0') {
                general_error("-o requires a path. use -o:path/to/out");
            }
            fl->output_path = a.val;
        } else if (string_eq(a.key, to_string("-timings"))) {
            fl->print_timings = true;
        } else {
            general_error("unrecognized option \""str_fmt"\"", str_arg(a.key));
        }
    }
}
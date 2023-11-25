#include "luna.h"
#include "lexer.h"
#include "core.h"

// ╭──────╮
// │ luna │ the Aphelion ISA assembler
// ╰──────╯
// by spsandwichman

int main(int argc, char* argv[]) {

    flag_set flags;
    load_arguments(argc, argv, &flags);

    FILE* asm_file = fopen(flags.input_path, "r");
    if (asm_file == NULL) {
        die("could not open file \"%s\"\n", flags.input_path);
        exit(EXIT_FAILURE);
    }
    char* asm_string = (char*) load_file(asm_file);
    fclose(asm_file);

    lexer_state lex;
    lexer_init(&lex, flags.input_path, asm_string, strlen(asm_string));

    while (lex.tokens.base[lex.tokens.len-1].type != tt_EOF) {
        append_next_token(&lex);
    }
    for (int i = 0; i < lex.tokens.len; i++) {
        print_token(&lex, &(lex.tokens.base[i]));
        printf(" ");
    }
}

void print_help() {
    printf("\nusage: luna (path) [flags]\n");
    printf("\n-o:[path]         specify an output path");
    printf("\n-help             display this text\n\n");
}

u8* load_file(FILE* asm_file) {
    fseek(asm_file, 0, SEEK_END);
    long file_size = ftell(asm_file);
    fseek(asm_file, 0, SEEK_SET);

    u8* asm_string = (u8*) malloc(file_size+1);
    fread(asm_string, file_size, 1, asm_file);
    asm_string[file_size] = '\0';
    return asm_string;
}

// hacky shit but whatever
cmd_arg make_argument(char* s) {
    for (size_t i = 0; s[i] != '\0'; i++) {
        if (s[i] == ':') {
            s[i] = '\0';
            return (cmd_arg){s, s+i+1};
        }
    }
    return (cmd_arg){s, ""};
}

void load_arguments(int argc, char* argv[], flag_set* fl) {
    if (argc < 2) {
        print_help();
        exit(EXIT_SUCCESS);
    }
    for (size_t i = 1; i < argc; i++) {
        cmd_arg a = make_argument(argv[i]);
        if (!strcmp(a.key, "-help")) {
            print_help();
            exit(EXIT_SUCCESS);
        } else if (!strcmp(a.key, "-o")) {
            fl->output_path = a.val;
        } else {
            if (i == 1 && a.key[0] != '-') {
                fl->input_path = a.key;
            } else {
                printf("error: unrecognized option \"%s\"\n", a.key);
                exit(EXIT_FAILURE);
            }
        }
    }
};
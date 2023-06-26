package luna

import "core:os"
import str "core:strings"
import "core:unicode/utf8"

// preprocessor
// takes the raw assembly and expands .inc, .def, and .mac
// JANK-ASS SHIT

preprocess :: proc(t: string) -> (res: string) {

    // expand tabs
    res = str.expand_tabs(t, 4)

    // .inc
    {
        included := make([dynamic]string)
        defer delete(included)
        for str.contains(res, ".inc") {
            for l, index in str.split_lines(res) {
                if !str.contains(l, ".inc") {
                    continue
                }

                line_tokens : [dynamic]btoken
                tokenize(l, &line_tokens) // lmfao invoking the lexer on a single line actually works pretty well

                if line_tokens[0].value != ".inc" || len(line_tokens) != 2 {
                    continue
                }

                inc_path := line_tokens[1].value[1:len(line_tokens[1].value)-1]

                inc_file, inc_readok := os.read_entire_file(inc_path)
                if !inc_readok {
                    die("ERR [line %d]: cannot read file at \"%s\"\n", index, inc_path)
                }
                
                if !in_dynarr(included, inc_path) || flag_keep_dup_inc {
                    r, _ := str.replace(res, l, transmute(string) inc_file, 1)
                    res = r
                    append(&included, inc_path)
                    break
                } else {
                    r, _ := str.remove(res, l, 1)
                    res = r
                    break
                }
            }
        }
    }

    // .def
    {
        defined := make([dynamic]string)
        defer delete(defined)
        for str.contains(res, ".def") {
            for l, index in str.split_lines(res) {
                if !str.contains(l, ".def") {
                    continue
                }

                line_tokens : [dynamic]btoken
                tokenize(l, &line_tokens) // lmfao invoking the lexer on a single line actually works pretty well

                if line_tokens[0].value != ".def" || len(line_tokens) != 3 {
                    continue
                }

                if in_dynarr(defined, line_tokens[1].value) {
                    die("ERR [line %d]: already defined \"%s\"\n", index, line_tokens[1].value)
                }

                append(&defined, line_tokens[1].value)

                line_removed, _ := str.remove(res, l, 1)
                res = replace_word_all(line_removed, line_tokens[1].value, line_tokens[2].value)
                delete(line_removed)
                delete(line_tokens)
                break   // restart definition search
            }
        }
    }

    //.mac
    {
        macros_defined := make([dynamic]macro)
        defer delete_macro_dynarr(macros_defined)
    }

    return
}

// jank ass shit - not actually too jank!!
replace_word_all :: proc(s, key, value: string) -> string {
    if !str.contains(s, key) {
        return ""
    }

    cursor := 0
    new_s := ""
    for str.contains(s[cursor:], key) {
        idx := str.index(s[cursor:], key) + cursor
        new_s = str.concatenate({new_s, s[cursor:idx]})
        if surrounded_by_seps(s, key, idx) {
            new_s = str.concatenate({new_s, value})
            cursor = idx + len(value)
        } else {
            new_s = str.concatenate({new_s, key})
            cursor = idx + len(key)
        }
    }
    new_s = str.concatenate({new_s, s[cursor:]})

    return new_s
}

surrounded_by_seps :: proc(s, key: string, index: int) -> bool {
    return (index == 0          || is_prep_separator(utf8.rune_at(s, index-1))) && 
           (index == len(key)-1 || is_prep_separator(utf8.rune_at(s, index+len(key))))
}

is_prep_separator :: proc(c: rune) -> bool {
    for r in prep_separators {
        if r == c do return true
    }
    return false
}

prep_separators :: [?]rune{' ', '\t', '\r', '\v', '\f', ',', '\n'}


in_dynarr :: proc(arr: [dynamic]string, str: string) -> bool {
    for i in arr {
        if str == i {
            return true
        }
    }
    return false
}

// todo make custom destructor for this bc this probably leaks memory
macro :: struct {
    name        : string,
    arguments   : [dynamic]string,
    body        : string,
}

delete_macro_dynarr :: proc(m: [dynamic]macro) {
    for _, i in m {
        delete_macro(m[i])
    }
    delete(m)
}

delete_macro :: proc(m: macro) {
    delete(m.name)
    delete(m.arguments)
    delete(m.body)
}
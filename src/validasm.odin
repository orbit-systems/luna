package luna

import "core:strings"

is_separator :: proc(c: rune) -> bool {
    for r in separators {
        if r == c do return true
    }
    return false
}

is_register :: proc(s: string) -> bool {
    // for r in registers {
    //     if r == strings.to_lower(s) do return true
    // }
    // return false
    return strings.to_lower(s) in registers
}

is_native_directive :: proc(s: string) -> bool {
    // for r in directives {
    //     if r == strings.to_lower(s) do return true
    // }
    // return false
    return strings.to_lower(s) in native_directives
}

is_native_instruction :: proc(s: string) -> bool {
    // for r in instructions {
    //     if r == strings.to_lower(s) do return true
    // }
    // return false
    return strings.to_lower(s) in native_instructions
}

// todo make custom destructor
statement :: struct {
    kind        : statement_kind,   // what kind of statement it is
    name        : string,           // name without formatting
    args        : [dynamic]argument,// arguments
    line        : u64,              // line number for error display

    opcode      : u8,               // opcode
    func        : u8,               // func
    rde         : u8,               // r1 for instruction encoding
    rs1         : u8,               // r2 for instruction encoding
    rs2         : u8,               // r3 for instruction encoding
    imm         : u64,              // imm for instruction encoding

    loc         : u64,              // location in image
    size        : u64,              // size of statement in bytes
}

argument :: struct {
    kind        : argument_kind,    // what kind of argument it is
    value_int   : u64,              // int value, if applicable
    value_str   : string,           // string value, if applicable
}

statement_kind :: enum {
    Unresolved = 0,
    Instruction,
    Directive,
    Label,
}

argument_kind :: enum {
    Unresolved = 0,
    Ignore,      // was useful for doing argument checking - might delete now that it's not really that useful
    Register,
    Integer,    // any single literal data point that isn't a string, eg raw hex, etc
    Symbol,
    Symbol_Offset,
    String,
}

comp_arg :: proc(a, b: argument_kind) -> bool {
    if a == ak.Integer && b == ak.Symbol_Offset ||
       b == ak.Integer && a == ak.Symbol_Offset {
        return true
       }

    return a == b
}

escape_seqs := map[string]string{
    "0"   = "\x00",
    "n"   = "\n",
    "\\"  = "\\",
    "\""  = "\"",
    "\'"  = "\'",
}

registers := map[string]int{
    "rz" = 0, 
    "ra" = 1, 
    "rb" = 2, 
    "rc" = 3, 
    "rd" = 4, 
    "re" = 5, 
    "rf" = 6, 
    "rg" = 7, 
    "rh" = 8, 
    "ri" = 9, 
    "rj" = 10, 
    "rk" = 11,    
    "pc" = 12, 
    "sp" = 13, 
    "fp" = 14, 
    "st" = 15,
}

separators      :: [?]rune{' ', '\t', '\r', '\v', '\f', ','}
separators_str  :: [?]string{" ", "\t", "\r", "\v", "\f", ","}

native_directives := map[string][]argument_kind{
    "u8"     = []ak{ak.Integer},
    "u16"    = []ak{ak.Integer},
    "u32"    = []ak{ak.Integer},
    "u64"    = []ak{ak.Integer},
    "i8"     = []ak{ak.Integer},
    "i16"    = []ak{ak.Integer},
    "i32"    = []ak{ak.Integer},
    "i64"    = []ak{ak.Integer},
    "u8be"   = []ak{ak.Integer},
    "u16be"  = []ak{ak.Integer},
    "u32be"  = []ak{ak.Integer},
    "u64be"  = []ak{ak.Integer},
    "i8be"   = []ak{ak.Integer},
    "i16be"  = []ak{ak.Integer},
    "i32be"  = []ak{ak.Integer},
    "i64be"  = []ak{ak.Integer},
    "byte"   = []ak{ak.Integer, ak.Integer},
    "string" = []ak{ak.String},
    "val"    = []ak{ak.Symbol},
    "bin"    = []ak{ak.String},

    "loc"    = []ak{ak.Integer},
    "skip"   = []ak{ak.Integer},
    "align"  = []ak{ak.Integer},
}

native_instruction :: struct {
    name    : string,
    args    : []argument_kind,
    fields  : []instruction_fmt_field,
    format  : instruction_fmt,
    opcode  : u8,
    func    : u8,
}

ak  :: argument_kind
fmt :: instruction_fmt
iff :: instruction_fmt_field

instruction_aliases := map[string][]string{
    "add"   = []string{"addi", "addr"},
    "adc"   = []string{"adci", "adcr"},
    "sub"   = []string{"subi", "subr"},
    "sbb"   = []string{"sbbi", "sbbr"},
    "mul"   = []string{"muli", "mulr"},
    "div"   = []string{"divi", "divr"},

    "and"   = []string{"andi", "andr"},
    "or"    = []string{"ori", "orr"},
    "nor"   = []string{"nori", "norr"},
    "xor"   = []string{"xori", "xorr"},
    "shl"   = []string{"shli", "shlr"},
    "asr"   = []string{"asri", "asrr"},
    "lsr"   = []string{"lsri", "lsrr"},
}

native_instructions := map[string]native_instruction{

/* --------------------------- no operation lmfao --------------------------- */

    "nop"   = native_instruction{
                name   = "nap",
                args   = {},
                fields = {},
                format = fmt.B,
                opcode = 0x0A,
                func   = 0,
            },

/* ----------------------------- system control ----------------------------- */

    "int"   = native_instruction{
                name   = "int",
                args   = {ak.Integer},
                fields = {iff.IMM},
                format = fmt.B,
                opcode = 0x10,
                func   = 0,
            },
    "inv"   = native_instruction{
                name   = "inv",
                args   = {},
                fields = {},
                format = fmt.M,
                opcode = 0x11,
                func   = 0,
            },
    "usr"   = native_instruction{
                name   = "usr",
                args   = {},
                fields = {},
                format = fmt.B,
                opcode = 0x12,
                func   = 0,
            },

/* ----------------------------- data transport ----------------------------- */

    "lli"   = native_instruction{
                name   = "lli",
                args   = {ak.Register,  ak.Integer},
                fields = {iff.RDE,      iff.IMM},
                format = fmt.F,
                opcode = 0x20,
                func   = 0,
            },
    "llis"  = native_instruction{
                name   = "llis",
                args   = {ak.Register,  ak.Integer},
                fields = {iff.RDE,      iff.IMM},
                format = fmt.F,
                opcode = 0x20,
                func   = 1,
            },
    "lui"   = native_instruction{
                name   = "lui",
                args   = {ak.Register,  ak.Integer},
                fields = {iff.RDE,      iff.IMM},
                format = fmt.F,
                opcode = 0x20,
                func   = 2,
            },
    "luis"  = native_instruction{
                name   = "luis",
                args   = {ak.Register,  ak.Integer},
                fields = {iff.RDE,      iff.IMM},
                format = fmt.F,
                opcode = 0x20,
                func   = 3,
            },
    "lti"   = native_instruction{
                name   = "lti",
                args   = {ak.Register,  ak.Integer},
                fields = {iff.RDE,      iff.IMM},
                format = fmt.F,
                opcode = 0x20,
                func   = 4,
            },
    "ltis"  = native_instruction{
                name   = "ltis",
                args   = {ak.Register,  ak.Integer},
                fields = {iff.RDE,      iff.IMM},
                format = fmt.F,
                opcode = 0x20,
                func   = 5,
            },
    "ltui"  = native_instruction{
                name   = "ltui",
                args   = {ak.Register,  ak.Integer},
                fields = {iff.RDE,      iff.IMM},
                format = fmt.F,
                opcode = 0x20,
                func   = 6,
            },
    "ltuis" = native_instruction{
                name   = "ltuis",
                args   = {ak.Register,  ak.Integer},
                fields = {iff.RDE,      iff.IMM},
                format = fmt.F,
                opcode = 0x20,
                func   = 7,
            },
    "lw"    = native_instruction{
                name   = "lw",
                args   = {ak.Register,  ak.Register,    ak.Integer},
                fields = {iff.RDE,      iff.RS1,        iff.IMM},
                format = fmt.M,
                opcode = 0x21,
                func   = 0,
            },
    "lbs"   = native_instruction{
                name   = "lbs",
                args   = {ak.Register,  ak.Register,    ak.Integer},
                fields = {iff.RDE,      iff.RS1,        iff.IMM},
                format = fmt.M,
                opcode = 0x22,
                func   = 0,
            },
    "lb"    = native_instruction{
                name   = "lb",
                args   = {ak.Register,  ak.Register,    ak.Integer},
                fields = {iff.RDE,      iff.RS1,        iff.IMM},
                format = fmt.M,
                opcode = 0x23,
                func   = 0,
            },
    "sw"    = native_instruction{
                name   = "sw",
                args   = {ak.Register,  ak.Register,    ak.Integer},
                fields = {iff.RDE,      iff.RS1,        iff.IMM},
                format = fmt.M,
                opcode = 0x24,
                func   = 0,
            },
    "sb"    = native_instruction{
                name   = "sb",
                args   = {ak.Register,  ak.Register,    ak.Integer},
                fields = {iff.RDE,      iff.RS1,        iff.IMM},
                format = fmt.M,
                opcode = 0x25,
                func   = 0,
            },
    "swp"   = native_instruction{
                name   = "swp",
                args   = {ak.Register,  ak.Register},
                fields = {iff.RDE,      iff.RS1},
                format = fmt.M,
                opcode = 0x26,
                func   = 0,
            },
    "mov"   = native_instruction{
                name   = "mov",
                args   = {ak.Register,  ak.Register},
                fields = {iff.RDE,      iff.RS1},
                format = fmt.M,
                opcode = 0x27,
                func   = 0,
            },

/* ------------------------------- arithmetic ------------------------------- */

    "addr"  = native_instruction{
                name   = "addr",
                args   = {ak.Register,  ak.Register,    ak.Register},
                fields = {iff.RDE,      iff.RS1,        iff.RS2},
                format = fmt.R,
                opcode = 0x30,
                func   = 0,
            },
    "addi"  = native_instruction{
                name   = "addi",
                args   = {ak.Register,  ak.Register,    ak.Integer},
                fields = {iff.RDE,      iff.RS1,        iff.IMM},
                format = fmt.M,
                opcode = 0x31,
                func   = 0,
            },
    "adcr"  = native_instruction{
                name   = "adcr",
                args   = {ak.Register,  ak.Register,    ak.Register},
                fields = {iff.RDE,      iff.RS1,        iff.RS2},
                format = fmt.R,
                opcode = 0x32,
                func   = 0,
            },
    "adci"  = native_instruction{
                name   = "adci",
                args   = {ak.Register,  ak.Register,    ak.Integer},
                fields = {iff.RDE,      iff.RS1,        iff.IMM},
                format = fmt.M,
                opcode = 0x33,
                func   = 0,
            },
    "subr"  = native_instruction{
                name   = "subr",
                args   = {ak.Register,  ak.Register,    ak.Register},
                fields = {iff.RDE,      iff.RS1,        iff.RS2},
                format = fmt.R,
                opcode = 0x34,
                func   = 0,
            },
    "subi"  = native_instruction{
                name   = "subi",
                args   = {ak.Register,  ak.Register,    ak.Integer},
                fields = {iff.RDE,      iff.RS1,        iff.IMM},
                format = fmt.M,
                opcode = 0x35,
                func   = 0,
            },
    "sbbr"  = native_instruction{
                name   = "sbbr",
                args   = {ak.Register,  ak.Register,    ak.Register},
                fields = {iff.RDE,      iff.RS1,        iff.RS2},
                format = fmt.R,
                opcode = 0x36,
                func   = 0,
            },
    "sbbi"  = native_instruction{
                name   = "sbbi",
                args   = {ak.Register,  ak.Register,    ak.Integer},
                fields = {iff.RDE,      iff.RS1,        iff.IMM},
                format = fmt.M,
                opcode = 0x37,
                func   = 0,
            },
    "mulr"  = native_instruction{
                name   = "mulr",
                args   = {ak.Register,  ak.Register,    ak.Register},
                fields = {iff.RDE,      iff.RS1,        iff.RS2},
                format = fmt.R,
                opcode = 0x38,
                func   = 0,
            },
    "muli"  = native_instruction{
                name   = "muli",
                args   = {ak.Register,  ak.Register,    ak.Integer},
                fields = {iff.RDE,      iff.RS1,        iff.IMM},
                format = fmt.M,
                opcode = 0x39,
                func   = 0,
            },
    "divr"  = native_instruction{
                name   = "divr",
                args   = {ak.Register,  ak.Register,    ak.Register},
                fields = {iff.RDE,      iff.RS1,        iff.RS2},
                format = fmt.R,
                opcode = 0x3a,
                func   = 0,
            },
    "divi"  = native_instruction{
                name   = "divi",
                args   = {ak.Register,  ak.Register,    ak.Integer},
                fields = {iff.RDE,      iff.RS1,        iff.IMM},
                format = fmt.M,
                opcode = 0x3b,
                func   = 0,
            },

/* ---------------------------------- logic --------------------------------- */

    "andr"  = native_instruction{
                name   = "andr",
                args   = {ak.Register,  ak.Register,    ak.Register},
                fields = {iff.RDE,      iff.RS1,        iff.RS2},
                format = fmt.R,
                opcode = 0x40,
                func   = 0,
            },
    "andi"  = native_instruction{
                name   = "andi",
                args   = {ak.Register,  ak.Register,    ak.Integer},
                fields = {iff.RDE,      iff.RS1,        iff.IMM},
                format = fmt.M,
                opcode = 0x41,
                func   = 0,
            },
    "orr"   = native_instruction{
                name   = "orr",
                args   = {ak.Register,  ak.Register,    ak.Register},
                fields = {iff.RDE,      iff.RS1,        iff.RS2},
                format = fmt.R,
                opcode = 0x42,
                func   = 0,
            },
    "ori"   = native_instruction{
                name   = "ori",
                args   = {ak.Register,  ak.Register,    ak.Integer},
                fields = {iff.RDE,      iff.RS1,        iff.IMM},
                format = fmt.M,
                opcode = 0x43,
                func   = 0,
            },
    "norr"  = native_instruction{
                name   = "norr",
                args   = {ak.Register,  ak.Register,    ak.Register},
                fields = {iff.RDE,      iff.RS1,        iff.RS2},
                format = fmt.R,
                opcode = 0x44,
                func   = 0,
            },
    "nori"  = native_instruction{
                name   = "nori",
                args   = {ak.Register,  ak.Register,    ak.Integer},
                fields = {iff.RDE,      iff.RS1,        iff.IMM},
                format = fmt.M,
                opcode = 0x45,
                func   = 0,
            },
    "xorr"  = native_instruction{
                name   = "xorr",
                args   = {ak.Register,  ak.Register,    ak.Register},
                fields = {iff.RDE,      iff.RS1,        iff.RS2},
                format = fmt.R,
                opcode = 0x46,
                func   = 0,
            },
    "xori"  = native_instruction{
                name   = "xori",
                args   = {ak.Register,  ak.Register,    ak.Integer},
                fields = {iff.RDE,      iff.RS1,        iff.IMM},
                format = fmt.M,
                opcode = 0x47,
                func   = 0,
            },
    "shlr"  = native_instruction{
                name   = "shlr",
                args   = {ak.Register,  ak.Register,    ak.Register},
                fields = {iff.RDE,      iff.RS1,        iff.RS2},
                format = fmt.R,
                opcode = 0x48,
                func   = 0,
            },
    "shli"  = native_instruction{
                name   = "shli",
                args   = {ak.Register,  ak.Register,    ak.Integer},
                fields = {iff.RDE,      iff.RS1,        iff.IMM},
                format = fmt.M,
                opcode = 0x49,
                func   = 0,
            },
    "asrr"  = native_instruction{
                name   = "asrr",
                args   = {ak.Register,  ak.Register,    ak.Register},
                fields = {iff.RDE,      iff.RS1,        iff.RS2},
                format = fmt.R,
                opcode = 0x4a,
                func   = 0,
            },
    "asri"  = native_instruction{
                name   = "asri",
                args   = {ak.Register,  ak.Register,    ak.Integer},
                fields = {iff.RDE,      iff.RS1,        iff.IMM},
                format = fmt.M,
                opcode = 0x4b,
                func   = 0,
            },
    "lsrr"  = native_instruction{
                name   = "lsrr",
                args   = {ak.Register,  ak.Register,    ak.Register},
                fields = {iff.RDE,      iff.RS1,        iff.RS2},
                format = fmt.R,
                opcode = 0x4c,
                func   = 0,
            },
    "lsri"  = native_instruction{
                name   = "lsri",
                args   = {ak.Register,  ak.Register,    ak.Integer},
                fields = {iff.RDE,      iff.RS1,        iff.IMM},
                format = fmt.M,
                opcode = 0x4d,
                func   = 0,
            },

/* ---------------------------------- stack --------------------------------- */

    "push"  = native_instruction{
                name   = "push",
                args   = {ak.Register},
                fields = {iff.RS1},
                format = fmt.M,
                opcode = 0x50,
                func   = 0,
            },
    "pushi" = native_instruction{
                name   = "pushi",
                args   = {ak.Integer},
                fields = {iff.IMM},
                format = fmt.M,
                opcode = 0x51,
                func   = 0,
            },
    "pushz" = native_instruction{
                name   = "pushz",
                args   = {ak.Integer},
                fields = {iff.IMM},
                format = fmt.M,
                opcode = 0x52,
                func   = 0,
            },
    "pushc" = native_instruction{
                name   = "pushc",
                args   = {ak.Integer},
                fields = {iff.IMM},
                format = fmt.M,
                opcode = 0x53,
                func   = 0,
            },
    "pop"   = native_instruction{
                name   = "pop",
                args   = {ak.Register},
                fields = {iff.RDE},
                format = fmt.M,
                opcode = 0x54,
                func   = 0,
            },
    "enter" = native_instruction{
                name   = "enter",
                args   = {},
                fields = {},
                format = fmt.B,
                opcode = 0x55,
                func   = 0,
            },
    "leave" = native_instruction{
                name   = "leave",
                args   = {},
                fields = {},
                format = fmt.B,
                opcode = 0x56,
                func   = 0,
            },
    "reloc" = native_instruction{
                name   = "reloc",
                args   = {ak.Register,  ak.Integer},
                fields = {iff.RS1,      iff.IMM},
                format = fmt.M,
                opcode = 0x57,
                func   = 0,
            },

/* ------------------------------ control flow ------------------------------ */

    "ljal"  = native_instruction{
                name   = "ljal",
                args   = {ak.Register,  ak.Integer},
                fields = {iff.RS1,      iff.IMM},
                format = fmt.M,
                opcode = 0x60,
                func   = 0,
            },
    "ljalr" = native_instruction{
                name   = ";jalr",
                args   = {ak.Register,  ak.Integer, ak.Register},
                fields = {iff.RS1,      iff.IMM,    iff.RDE},
                format = fmt.M,
                opcode = 0x61,
                func   = 0,
            },
    "ret"   = native_instruction{
                name   = "ret",
                args   = {},
                fields = {},
                format = fmt.F,
                opcode = 0x62,
                func   = 0,
            },
    "retr"  = native_instruction{
                name   = "retr",
                args   = {ak.Register},
                fields = {iff.RDE},
                format = fmt.F,
                opcode = 0x63,
                func   = 0,
            },
    "jal"   = native_instruction{
                name   = "jal",
                args   = {ak.Symbol},
                fields = {iff.IMM},
                format = fmt.J,
                opcode = 0x65,
                func   = 0,
            },
    "jalr"  = native_instruction{
                name   = "jalr",
                args   = {ak.Symbol, ak.Register},
                fields = {iff.IMM,    iff.RDE},
                format = fmt.J,
                opcode = 0x66,
                func   = 0,
            },
    
    "bra"   = native_instruction{
                name   = "bra",
                args   = {ak.Symbol},
                fields = {iff.IMM},
                format = fmt.B,
                opcode = 0x64,
                func   = 0x0,
            },
    "beq"   = native_instruction{
                name   = "beq",
                args   = {ak.Symbol},
                fields = {iff.IMM},
                format = fmt.B,
                opcode = 0x64,
                func   = 0x1,
            },
    "bez"   = native_instruction{
                name   = "bez",
                args   = {ak.Symbol},
                fields = {iff.IMM},
                format = fmt.B,
                opcode = 0x64,
                func   = 0x2,
            },
    "blt"   = native_instruction{
                name   = "blt",
                args   = {ak.Symbol},
                fields = {iff.IMM},
                format = fmt.B,
                opcode = 0x64,
                func   = 0x3,
            },
    "ble"   = native_instruction{
                name   = "ble",
                args   = {ak.Symbol},
                fields = {iff.IMM},
                format = fmt.B,
                opcode = 0x64,
                func   = 0x4,
            },
    "bltu"  = native_instruction{
                name   = "bltu",
                args   = {ak.Symbol},
                fields = {iff.IMM},
                format = fmt.B,
                opcode = 0x64,
                func   = 0x5,
            },
    "bleu"  = native_instruction{
                name   = "bleu",
                args   = {ak.Symbol},
                fields = {iff.IMM},
                format = fmt.B,
                opcode = 0x64,
                func   = 0x6,
            },
    "bpe"   = native_instruction{
                name   = "bpe",
                args   = {ak.Symbol},
                fields = {iff.IMM},
                format = fmt.B,
                opcode = 0x64,
                func   = 0x7,
            },
    "bne"   = native_instruction{
                name   = "bne",
                args   = {ak.Symbol},
                fields = {iff.IMM},
                format = fmt.B,
                opcode = 0x64,
                func   = 0x9,
            },
    "bnz"   = native_instruction{
                name   = "bnz",
                args   = {ak.Symbol},
                fields = {iff.IMM},
                format = fmt.B,
                opcode = 0x64,
                func   = 0xa,
            },
    "bge"   = native_instruction{
                name   = "bge",
                args   = {ak.Symbol},
                fields = {iff.IMM},
                format = fmt.B,
                opcode = 0x64,
                func   = 0xb,
            },
    "bgt"   = native_instruction{
                name   = "bgt",
                args   = {ak.Symbol},
                fields = {iff.IMM},
                format = fmt.B,
                opcode = 0x64,
                func   = 0xc,
            },
    "bgeu"  = native_instruction{
                name   = "bgeu",
                args   = {ak.Symbol},
                fields = {iff.IMM},
                format = fmt.B,
                opcode = 0x64,
                func   = 0xd,
            },
    "bgtu"  = native_instruction{
                name   = "bgtu",
                args   = {ak.Symbol},
                fields = {iff.IMM},
                format = fmt.B,
                opcode = 0x64,
                func   = 0xe,
            },
    "bpd"   = native_instruction{
                name   = "bpd",
                args   = {ak.Symbol},
                fields = {iff.IMM},
                format = fmt.B,
                opcode = 0x64,
                func   = 0xf,
            },
}

// ins :: enum {
//     nop,
//     int,
//     inv,
//     usr,

//     lli,
//     llis,
//     lui,
//     luis,
//     lti,
//     ltis,
//     ltui,
//     ltuis,
//     lw
// }

instruction_fmt :: enum {
    R, M, F, J, B,
}

instruction_fmt_field :: enum {
    RDE, RS1, RS2, IMM,
}
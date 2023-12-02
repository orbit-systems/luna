At the top level, assembly files can only have directives, no assembly. 
If inside a section where assembly is allowed, directives must be prefixed with a period `.`. They can also be prefixed with periods outside of assembly sections, but it is not required.

NOTE: Parenthesis `()` indicates a required argument. brackets `[]` indicate an optional argument.

NOTE: commas are generally optional between arguments/lists. 
if they are not, it will be indicated.

# directives

## symbol data
directives that manipulate symbols do not have to be in order. For example, a symbol may be bound global before it is defined/given a value. However, this requires the symbol be declared somewhere in the object file (no changing the attributes of symbols that don't exist lol).

- `define (symbol), (int)` - declare a symbol and set it to a specific value.
- `bind (symbol), (type)` - set a symbol's visiblity. Choose from either `global`, `weak` or `local`. A symbol's default visibility is `local`.
- `type (symbol), (type)` - set a symbol's associated type. Choose from either `function`, `data`, or `untyped`. A symbol's default type is `untyped`.
- `size (symbol), (int)` - set a symbol's associated size in bytes.

## sections
there are 3 kinds of user-definable sections.

- `section (name) [perms] {...}` - this contains assembly or data. the name of the section must be inside a string. any sections named `"code"` will automatically have read and execute permissions. any sections named `"data"` will automatically have read and write permissions. any sections named `"rodata"` will just have read permissions. you may optionally specify custom permissions after the name string. if the string contains `r` or `R`, the section has read permissions. if it contains `w` or `W`, the section has write permissions. if the string contains `x` or `X`, the section has execute permissions. if the section has no default permissions (derived from the section name) and no custom permissions, it will have all (`rwx`) permissions.
- `blank (name) [perms] {...}` - this should contain uninitialized (zeroed) data. the string is the name of the section. it will automatically have read and write permissions, if not given custom permissions. Any non-zero data/instructions will generate a warning, as these will be zeroed.
- `info {...}` - this contains arbitrary key-value metadata that you may want to store, such as debug information, compiler info, etc. There can be multiple of these sections, and they will be coalesced into one in the final object file.

# pseudo-instructions

in addition to ISA-defined pseudo-instructions/aliases, these are supplied:

- `align (int)` - skip forward (fill with zero) until the cursor position is aligned to a multiple of (int). A warning will be generated if the argument is not a power of two. Real instructions are automatically aligned to multiples of 4.

- `u8 (int), [count]` - Repeat a u8 value `(int)`, `[count]` # of times. if `[count]` is not present, it defaults to 1.

- `i8 (int), [count]` - Repeat an i8 value `(int)`, `[count]` # of times. if `[count]` is not present, it defaults to 1.

- `u16 (int), [count]` - Repeat a u16 value `(int)`, `[count]` # of times. if `[count]` is not present, it defaults to 1. Automatically aligned to multiple of 2.

- `i16 (int), [count]` - Repeat an i16 value `(int)`, `[count]` # of times. if `[count]` is not present, it defaults to 1. Automatically aligned to multiple of 2.

- `u32 (int), [count]` - Repeat a u32 value `(int)`, `[count]` # of times. if `[count]` is not present, it defaults to 1. Automatically aligned to multiple of 4.

- `i32 (int), [count]` - Repeat an i32 value `(int)`, `[count]` # of times. if `[count]` is not present, it defaults to 1. Automatically aligned to multiple of 4.

- `u64 (int), [count]` - Repeat a u64 value `(int)`, `[count]` # of times. if `[count]` is not present, it defaults to 1. Automatically aligned to multiple of 8.

- `i64 (int), [count]` - Repeat an i64 value `(int)`, `[count]` # of times. if `[count]` is not present, it defaults to 1. Automatically aligned to multiple of 8.

NOTE: all integers except `u8` and `i8` have big-endian counterparts, just add `be` to the end of the pseudo-instruction.

- `skip (int)` - equivalent to `u8 0, (int)`.

- `utf8 (string)` - Inserts a UTF-8 encoded string. Does not automatically null-terminate.

- `utf16 (string)` - Inserts a UTF-16 encoded string. Does not automatically null-terminate.

- `utf32 (string)` - Inserts a UTF-32 encoded string. Does not automatically null-terminate.

NOTE: `utf32` and `utf16` also have big-endian counterparts, just add `be` to the end of the pseudo-instruction.
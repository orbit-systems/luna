at the top level, assembly files can only have directives, no assembly. 
if inside a section where assembly is allowed, directives must be prefixed with a period `.`. They can also be prefixed with periods outside of assembly sections, but it is not required.

NOTE: commas are generally optional between arguments/lists. 
if they are not, it will be indicated.

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
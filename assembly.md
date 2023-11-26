at the top level, assembly files can only have directives, no assembly. 
if inside a section where assembly is allowed, directives must be prefixed with a period `.`.

NOTE: commas are generally optional between arguments/lists. 
if they are not, it will be indicated.

## object data

- `from (string)` - sets the object file's internal source file name, which would otherwise default to the name of the assembly file.

## symbol data
directives that manipulate symbols do not have to be in order. For example, a symbol may be bound global before it is defined/given a value. However, this requires the symbol be declared somewhere in the object file (no changing the attributes of symbols that don't exist lol).

- `declare (symbol), (int)` - declare a symbol and set it to a specific value.
- `bind (symbol), (type)` - set a symbol's visiblity. Choose from either `global`, `weak` or `local`. A symbol's default visibility is `local`.
- `weak (symbol)` - set a symbol's visiblity to .
- `type (symbol), (type)` - set a symbol's associated type. Choose from either `function`, `data`, or `untyped`. A symbol's default type is `untyped`.
- `size (symbol), (int)` - set a symbol's associated size in bytes.

## sections
there are 3 kinds of user-definable sections.

- `section (string) {...}` - this contains assembly or data. the string is the name. Any sections named
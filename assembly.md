NOTE: Parenthesis `()` indicates a required argument. brackets `[]` indicate an optional argument.

# values

there are several types of literals:

- standard int literals, like `10`.
- floating point literals, like `10.0f`. They must be postfixed with `h`, `f`, or `d` for half-, single-, and double-precision literals.
- character literals, like `'A'`. Their value is the ASCII value of the character. common escape sequences are supported.
- string literals, like `"Hello"`. They can only be used with directives or pseudo-instructions that accept them.

# labels

labels are defined like so:
```nasm
label:
```
the label must have a newline after it.
if the label is prefixed with `.`, it is a local label. it will internally be prefixed with with the name of the last non-local label. for example:

```nasm
some_function:
    li ra, 0
    .loop:          ; internally "some_function.loop:"
        add ra, ra, 1
        cmp ra, 10
        bne .loop;  ; internally "bne some_function.loop"
    ret
```

# directives

## symbol data

- `define (symbol), (int)` - declare a symbol and set it to a specific value.

# pseudo-instructions

In addition to ISA-defined pseudo-instructions, these are supplied:

- `loc (int), [fill]` - set the address cursor to `(int)`, filling the empty space will `[fill]`. Empty space is not overwritten if `[fill]` is not present OR if `loc` sets the address cursor backwards.

- `align (int), [fill]` - Insert byte `[fill]` until the cursor position is aligned to a multiple of `(int)`. An error will be generated if the argument is not a power of two. If `[fill]` is not present, empty space will not be overwritten.

- `byte (val), [count]` - Alias for `d8`.

- `d8 (val), [count]` - Repeat an 8-bit value `(val)`, `[count]`(default to 1) # of times.

- `d16 (val), [count]` - Repeat a 16-bit value `(val)`, `[count]`(default to 1) # of times. Errors if not aligned to multiple of 2.

- `d32 (val), [count]` - Repeat a 32-bit value `(val)`, `[count]`(default to 1) # of times. Errors if not aligned to multiple of 4.

- `d64 (val), [count]` - Repeat a 64-bit value `(val)`, `[count]`(default to 1) # of times. Errors if not aligned to multiple of 8.

- `skip (val)` - equivalent to `d8 0, (val)`.

- `utf8 (string)` - Inserts a UTF-8 encoded string. Does not automatically null-terminate.

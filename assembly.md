NOTE: Parenthesis `()` indicates a required argument. brackets `[]` indicate an optional argument.

# labels

labels are defined like so:

`label:`

the label must have a newline after it.
if the label is prefixed with `.`, it is a local label. it will internally be prefixed with with the name of the last non-local label. for example:

```arm
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

in addition to ISA-defined pseudo-instructions/aliases, these are supplied:

- `loc (int), [fill]` - skip forward until address `(int)`, filling the empty space will `[fill]`.

- `align (int), [fill]` - insert byte `[fill]`(defaults to 0) until the cursor position is aligned to a multiple of `(int)`. A warning will be generated if the argument is not a power of two. Real instructions are automatically aligned to multiples of 4.

- `byte (int), [count]` - alias for `u8`

- `d8 (int), [count]` - Repeat a u8 value `(int)`, `[count]`(default to 1) # of times.

- `d16 (int), [count]` - Repeat an i16 value `(int)`, `[count]`(default to 1) # of times. Automatically aligned to multiple of 2.

- `d32 (int), [count]` - Repeat a 32-bit value `(int)`, `[count]`(default to 1) # of times. Automatically aligned to multiple of 4.

- `d64 (int), [count]` - Repeat a 64-bit value `(int)`, `[count]`(default to 1) # of times. Automatically aligned to multiple of 8.

- `skip (int)` - equivalent to `u8 0, (int)`.

- `utf8 (string)` - Inserts a UTF-8 encoded string. Does not automatically null-terminate.

- `utf16 (string)` - Inserts a UTF-16 encoded string. Does not automatically null-terminate.

- `utf32 (string)` - Inserts a UTF-32 encoded string. Does not automatically null-terminate.

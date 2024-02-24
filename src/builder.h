#pragma once
#define BUILDER_H

#include "orbit.h"
#include "parser.h"

#define sign_extend(val, bitsize) ((u64)((i64)((u64)val << (64-bitsize)) >> (64-bitsize)))
#define zero_extend(val, bitsize) ((u64)((u64)((u64)val << (64-bitsize)) >> (64-bitsize)))

// figures out if a value can be losslessly compressed into a bitwidth integer
#define can_losslessly_signext(value, bitwidth) ((value) == sign_extend((value), (bitwidth)))
#define can_losslessly_zeroext(value, bitwidth) ((value) == zero_extend((value), (bitwidth)))

// check arguments and expand macros
void check_and_expand(luna_file* restrict f);

// return size of a translated binary
u64 trace_size(luna_file* restrict f);
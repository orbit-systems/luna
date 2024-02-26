#pragma once
#define BUILDER_H

#include "orbit.h"
#include "parser.h"

u64  trace_size(luna_file* restrict f);
void emit_binary(luna_file* restrict f, void* bin);
aphel_instruction encode_instruction(luna_file* restrict f, element* restrict e, u64 position);
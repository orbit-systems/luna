#pragma once
#define LUNA_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

// not gonna use stdbool fuck you
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int64_t  i64;
typedef int32_t  i32;
typedef int16_t  i16;
typedef int8_t   i8;
typedef uint64_t b64;
typedef uint32_t b32;
typedef uint16_t b16;
typedef uint8_t  b8;
typedef uint8_t  bool;
#define false 0
#define true 1

#define U64_MAX (u64)0xFFFFFFFFFFFFFFFFull
#define U64_MIN (u64)0ull
#define I64_MAX (i64)0x7FFFFFFFFFFFFFFFll
#define I64_MIN (i64)0x8000000000000000ll

#define TODO(msg) \
    printf("TODO: \"%s\" at %s:%d\n", (msg), (__FILE__), (__LINE__)); \
    exit(EXIT_FAILURE)

#define die(msg, ...) \
    printf("ERROR: "); \
    printf(msg __VA_OPT__(,) __VA_ARGS__); \
    exit(EXIT_FAILURE)
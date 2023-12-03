#pragma once
#define LUNA_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <limits.h>
#include <stdarg.h>

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

#define TODO(msg) do {\
    printf("TODO: \"%s\" at %s:%d\n", (msg), (__FILE__), (__LINE__)); exit(EXIT_FAILURE); } while (0);

// crash() indicates something in the compiler's internals has gone wrong and needs to be fixed
#define crash(msg, ...) do {\
    printf("CRASH at %s:%d - ", (__FILE__), (__LINE__)); \
    printf(msg __VA_OPT__(,) __VA_ARGS__); \
    exit(EXIT_FAILURE); } while (0);

/*
typedef struct string_s {
    char* base;
    size_t len;
} string;
*/
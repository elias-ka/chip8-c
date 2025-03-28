#ifndef CHIP8_UTIL_H
#define CHIP8_UTIL_H

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*arr))

void *xcalloc(size_t n, size_t size);
noreturn void die(const char *fmt, ...);

#define malloc(a) USE_XCALLOC_INSTEAD
#define calloc(a, b) USE_XCALLOC_INSTEAD
#endif

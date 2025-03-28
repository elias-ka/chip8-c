#include "util.h"

#undef malloc
#undef calloc

void *xcalloc(size_t n, size_t size)
{
    void *p = calloc(n, size);
    if (!p && size) {
        abort();
    }
    return p;
}

noreturn void die(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    va_end(args);
    exit(EXIT_FAILURE);
}

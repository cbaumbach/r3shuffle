#include "MemorySpy.h"
#include <stddef.h>
#include <stdio.h>

#define MAXALLOCSIZE 32

static char buf[MAXALLOCSIZE];
static char *buf_beg = NULL;
static char *buf_end = NULL;
static char *buf_cur = NULL;

static int Malloc_ReturnNULL = 0;

void *MemorySpy_Malloc(size_t nbytes)
{
    int i;
    char *s;

    if (Malloc_ReturnNULL) {
        Malloc_ReturnNULL = 0;
        return NULL;
    }

    if (buf_beg == NULL || buf_end == NULL || buf_cur == NULL)
        MemorySpy_MallocReset();

    if (nbytes > (size_t) (buf_end - buf_cur))
        return NULL;

    s = buf_cur;
    buf_cur += nbytes;

    for (i = 0; i < (int) nbytes; i++)
        s[i] = i;

    return s;
}

void MemorySpy_MallocReset(void)
{
    buf_beg = buf_cur = buf;
    buf_end = buf + sizeof buf;
}

void MemorySpy_MallocReturnNULL(void)
{
    Malloc_ReturnNULL = 1;
}

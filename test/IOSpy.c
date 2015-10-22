#include "IOSpy.h"
#include <stddef.h>
#include <stdio.h>

static int OpenFile_ReturnNULL = 0;
static FILE FakeFILE;

FILE *IOSpy_OpenFile(const char *filename, const char *mode)
{
    if (OpenFile_ReturnNULL) {
        OpenFile_ReturnNULL = 0;
        return NULL;
    }

    return &FakeFILE;  // non-NULL
}

void IOSpy_OpenFileReturnNULL(void)
{
    OpenFile_ReturnNULL = 1;
}

void IOSpy_OpenFileReturnNonNULL(void)
{
    OpenFile_ReturnNULL = 0;
}

int IOSpy_CloseFile(FILE *fp)
{
    return EOF;
}

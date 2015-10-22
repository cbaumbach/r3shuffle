#include "Stream.h"
#include "IO.h"
#include "Memory.h"
#include <stddef.h>
#include <stdio.h>

struct StreamStruct {
    int chunk_size;
};

Stream Stream_Create(const char *filename)
{
    FILE *fp;
    Stream st;

    if ((fp = IO_OpenFile(filename, "rb")) == NULL)
        return NULL;

    if ((st = (Stream) Memory_Malloc(sizeof(*st))) == NULL)
        goto CLOSE_FILE;

    st->chunk_size = 1;

    return st;

CLOSE_FILE:
    IO_CloseFile(fp);

    return NULL;
}

int Stream_GetChunkSize(Stream st)
{
    return st->chunk_size;
}

void Stream_SetChunkSize(Stream st, int chunk_size)
{
    st->chunk_size = chunk_size;
}

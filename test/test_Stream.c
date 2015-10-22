#include "unity_fixture.h"
#include "Stream.h"
#include "IO.h"
#include "IOSpy.h"
#include "Memory.h"
#include "MemorySpy.h"
#include <stddef.h>
#include <stdio.h>

static Stream stream;
static const char *filename = "foobar";
static const char *line = NULL;

static FILE *(*Old_IO_OpenFile)(const char *, const char *) = NULL;
static int (*Old_IO_CloseFile)(FILE *) = NULL;
static void *(*Old_Memory_Malloc)(size_t nbytes) = NULL;

TEST_GROUP(Stream);

TEST_SETUP(Stream)
{
    Old_IO_OpenFile = IO_OpenFile;
    IO_OpenFile = IOSpy_OpenFile;

    Old_IO_CloseFile = IO_CloseFile;
    IO_CloseFile = IOSpy_CloseFile;

    Old_Memory_Malloc = Memory_Malloc;
    Memory_Malloc = MemorySpy_Malloc;

    stream = Stream_Create(filename);
}

TEST_TEAR_DOWN(Stream)
{
    IO_OpenFile = Old_IO_OpenFile;
    IO_CloseFile = Old_IO_CloseFile;
    Memory_Malloc = Old_Memory_Malloc;

    MemorySpy_MallocReset();
}

TEST(Stream, return_null_if_file_cannot_be_opened)
{
    Stream st;

    IOSpy_OpenFileReturnNULL();
    st = Stream_Create(filename);

    TEST_ASSERT_TRUE(st == NULL);
}

TEST(Stream, return_non_null_if_everything_works)
{
    TEST_ASSERT_TRUE(stream != NULL);
}

TEST(Stream, default_chunk_size_is_one)
{
    TEST_ASSERT_EQUAL_INT(1, Stream_GetChunkSize(stream));
}

TEST(Stream, chunk_size_can_be_changed)
{
    Stream_SetChunkSize(stream, 3);
    TEST_ASSERT_EQUAL_INT(3, Stream_GetChunkSize(stream));
}

TEST(Stream, chunks_sizes_of_different_streams_are_independent)
{
    Stream another_stream = Stream_Create(filename);

    Stream_SetChunkSize(stream, 3);
    Stream_SetChunkSize(another_stream, 4);
    TEST_ASSERT_EQUAL_INT(3, Stream_GetChunkSize(stream));
    TEST_ASSERT_EQUAL_INT(4, Stream_GetChunkSize(another_stream));
}

TEST(Stream, return_null_if_malloc_fails)
{
    Stream st;

    MemorySpy_MallocReturnNULL();
    st = Stream_Create(filename);

    TEST_ASSERT_TRUE(st == NULL);
}

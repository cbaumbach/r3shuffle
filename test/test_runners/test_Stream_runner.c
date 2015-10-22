#include "unity_fixture.h"

TEST_GROUP_RUNNER(Stream)
{
    RUN_TEST_CASE(Stream, return_null_if_file_cannot_be_opened);
    RUN_TEST_CASE(Stream, return_non_null_if_everything_works);
    RUN_TEST_CASE(Stream, default_chunk_size_is_one);
    RUN_TEST_CASE(Stream, chunk_size_can_be_changed);
    RUN_TEST_CASE(Stream, chunks_sizes_of_different_streams_are_independent);
    RUN_TEST_CASE(Stream, return_null_if_malloc_fails);
}

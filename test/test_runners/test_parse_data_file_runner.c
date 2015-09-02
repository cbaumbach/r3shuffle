#include "unity_fixture.h"

TEST_GROUP_RUNNER(parse_data_file)
{
    RUN_TEST_CASE(parse_data_file, offset2index);
    RUN_TEST_CASE(parse_data_file, index2offset);
    RUN_TEST_CASE(parse_data_file, index2offset_is_reverse_of_offset2index);
    RUN_TEST_CASE(parse_data_file, offset2index_is_reverse_of_index2offset);
}

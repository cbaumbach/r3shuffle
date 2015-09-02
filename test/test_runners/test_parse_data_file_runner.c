#include "unity_fixture.h"

TEST_GROUP_RUNNER(parse_data_file)
{
    RUN_TEST_CASE(parse_data_file, offset2index);
    RUN_TEST_CASE(parse_data_file, index2offset);
}

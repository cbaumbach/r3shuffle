#include "unity_fixture.h"

static void RunAllTests(void)
{
    RUN_TEST_GROUP(parse_command_line_args);
    RUN_TEST_GROUP(parse_layout_file);
    RUN_TEST_GROUP(parse_data_file);
    RUN_TEST_GROUP(Stream);
}

int main(int argc, const char *argv[])
{
    return UnityMain(argc, argv, RunAllTests);
}

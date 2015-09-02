#include "unity_fixture.h"

TEST_GROUP_RUNNER(parse_command_line_args)
{
    RUN_TEST_CASE(parse_command_line_args, long_help);
    RUN_TEST_CASE(parse_command_line_args, short_help);
    RUN_TEST_CASE(parse_command_line_args, long_digits_without_equal);
    RUN_TEST_CASE(parse_command_line_args, long_digits_with_equal);
    RUN_TEST_CASE(parse_command_line_args, short_digits);
    RUN_TEST_CASE(parse_command_line_args, bad_digits_gives_error);
    RUN_TEST_CASE(parse_command_line_args, digits_over_100_gives_error);
    RUN_TEST_CASE(parse_command_line_args, digits_under_0_gives_error);
    RUN_TEST_CASE(parse_command_line_args, output_file_is_set);
    RUN_TEST_CASE(parse_command_line_args, column_labels_are_set);
    RUN_TEST_CASE(parse_command_line_args, missing_column_label_argument_gives_error);
    RUN_TEST_CASE(parse_command_line_args, print_columns_is_set);
    RUN_TEST_CASE(parse_command_line_args, input_files_are_set);
    RUN_TEST_CASE(parse_command_line_args, missing_input_files_give_error);
    RUN_TEST_CASE(parse_command_line_args, non_writable_output_file_gives_error);
    RUN_TEST_CASE(parse_command_line_args, output_file_defaults_to_stdout);
    RUN_TEST_CASE(parse_command_line_args, existing_writable_output_file);
    RUN_TEST_CASE(parse_command_line_args, non_existing_input_files_give_error);
    RUN_TEST_CASE(parse_command_line_args, non_existing_layout_file_gives_error);
    RUN_TEST_CASE(parse_command_line_args, non_existing_data_file_gives_error);
    RUN_TEST_CASE(parse_command_line_args, non_existing_but_writable_output_file_gone_after_test);
    RUN_TEST_CASE(parse_command_line_args, empty_output_filename_gives_error);
}

#include "unity_fixture.h"

TEST_GROUP_RUNNER(parse_layout_file)
{
    RUN_TEST_CASE(parse_layout_file, write_and_read_back_layout_file);
    RUN_TEST_CASE(parse_layout_file, bad_magic_number);
    RUN_TEST_CASE(parse_layout_file, bad_bytes_per_double);
    RUN_TEST_CASE(parse_layout_file, bad_number_of_covariates);
    RUN_TEST_CASE(parse_layout_file, non_positive_number_of_snps_causes_error_during_write);
    RUN_TEST_CASE(parse_layout_file, non_positive_number_of_traits_causes_error_during_write);
    RUN_TEST_CASE(parse_layout_file, non_positive_number_of_covariates_causes_error_during_write);
    RUN_TEST_CASE(parse_layout_file, max_char_smaller_than_2_causes_error_during_write);
    RUN_TEST_CASE(parse_layout_file, bad_number_of_snps);
    RUN_TEST_CASE(parse_layout_file, bad_number_of_traits);
    RUN_TEST_CASE(parse_layout_file, bad_number_of_snps_per_tile);
    RUN_TEST_CASE(parse_layout_file, bad_number_of_traits_per_tile);
    RUN_TEST_CASE(parse_layout_file, bad_max_label_length);
    RUN_TEST_CASE(parse_layout_file, set_column_print_order_with_subset_of_columns);
    RUN_TEST_CASE(parse_layout_file, set_permuted_column_print_order);
    RUN_TEST_CASE(parse_layout_file, invalid_user_supplied_column_label);
    RUN_TEST_CASE(parse_layout_file, use_default_columns);
}

#include "unity_fixture.h"
#include "parse_command_line_args.h"
#include "err_msg.h"
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define NELEMS(x) (sizeof (x) / sizeof (x[0]))

static struct Params params;
static int status;

TEST_GROUP(parse_command_line_args);

TEST_SETUP(parse_command_line_args)
{
    initialize_parameters(&params);
    optind = 1;                 /* reset getopt_long */
}

TEST_TEAR_DOWN(parse_command_line_args)
{
}

/* Test that long --help option is recognized. */
TEST(parse_command_line_args, long_help)
{
    char *argv[] = {"ignore", "--help"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, params.help, "help");
}

/* Test that short -h option is recognized. */
TEST(parse_command_line_args, short_help)
{
    char * argv[] = {"ignore", "-h"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, params.help, "help");
}

/* Test that long --digits option without equal sign is recognized. */
TEST(parse_command_line_args, long_digits_without_equal)
{
    char *argv[] = {"ignore", "--digits", "3"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");
    TEST_ASSERT_EQUAL_INT_MESSAGE(3, params.ndigit, "ndigit");
}

/* Test that long --digits option with equal sign is recognized. */
TEST(parse_command_line_args, long_digits_with_equal)
{
    char *argv[] = {"ignore", "--digits=4"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");
    TEST_ASSERT_EQUAL_INT_MESSAGE(4, params.ndigit, "ndigit");
}

/* Test that short -d option is recognized. */
TEST(parse_command_line_args, short_digits)
{
    char *argv[] = {"ignore", "-d4"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");
    TEST_ASSERT_EQUAL_INT_MESSAGE(4, params.ndigit, "ndigit");
}

/* Test that an all-alphabetical argument to --digits results in an
   error message. */
TEST(parse_command_line_args, bad_digits_gives_error)
{
    char *argv[] = {"ignore", "--digits", "foo"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status, "parse status");
    TEST_ASSERT_EQUAL_STRING("failed to convert --digits to integer: "
        "foo", err_msg);
}

/* Test that supplying a number >= 100 as the argument to --digits
   causes an error. */
TEST(parse_command_line_args, digits_over_100_gives_error)
{
    char *argv[] = {"ignore", "--digits=100"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");

    clear_err_msg();
    status = validate_command_line_args(&params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status, "validate status");
    TEST_ASSERT_EQUAL_STRING("argument to --digits must be <100",
        err_msg);
}

/* Test that supplying a number < 0 as the argument to --digits causes
   an error. */
TEST(parse_command_line_args, digits_under_0_gives_error)
{
    char *argv[] = {"ignore", "--digits=-10"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");

    clear_err_msg();
    status = validate_command_line_args(&params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status, "validate status");
    TEST_ASSERT_EQUAL_STRING("argument to --digits must be >=0",
        err_msg);
}

/* Test that output file gets set correctly. */
TEST(parse_command_line_args, output_file_is_set)
{
    char *argv[] = {"ignore", "--output", "foo"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");
    TEST_ASSERT_EQUAL_STRING("foo", params.output_file);
}

/* Test that user-supplied column labels are detected and saved in the
   correct order. */
TEST(parse_command_line_args, column_labels_are_set)
{
    char *argv[] = {
        "ignore", "--column", "foo", "--digits", "3", "--column=bar",
        "--help", "-cbaz", "--help", "-c", "quux"
    };
    int i;

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");
    TEST_ASSERT_EQUAL_INT(4, params.ncolumn);
    TEST_ASSERT_EQUAL_STRING("foo", params.columns[0]);
    TEST_ASSERT_EQUAL_STRING("bar", params.columns[1]);
    TEST_ASSERT_EQUAL_STRING("baz", params.columns[2]);
    TEST_ASSERT_EQUAL_STRING("quux", params.columns[3]);

    TEST_ASSERT_TRUE(params.ucp2acp != NULL);
    for (i = 0; i < 4; i++)
        TEST_ASSERT_EQUAL_INT(-1, params.ucp2acp[i]);
    TEST_ASSERT_EQUAL_INT(-9, params.ucp2acp[i]);
}

/* Test that forgetting to supply a column label after --column causes
   an error. */
TEST(parse_command_line_args,
    missing_column_label_argument_gives_error)
{
    char *argv[] = {"ignore", "--column"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status, "parse status");
    TEST_ASSERT_EQUAL_STRING("missing argument: --column", err_msg);
}

/* Test that --print-columns is recognized. */
TEST(parse_command_line_args, print_columns_is_set)
{
    char *argv[] = {"ignore", "--print-columns"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, params.print_columns,
        "print_columns");
}

/* Test that input files are set correctly. */
TEST(parse_command_line_args, input_files_are_set)
{
    char *argv[] = {"ignore", "foo/bar"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("foo/bar.iout",
        params.layout_file, "layout file");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("foo/bar.out",
        params.data_file, "data file");
}

/* Test that forgetting to supply an input file lead to an error. */
TEST(parse_command_line_args, missing_input_files_give_error)
{
    char *argv[] = {"ignore", "-otest/data/output.txt"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");

    clear_err_msg();
    status = validate_command_line_args(&params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status, "validate status");
    TEST_ASSERT_EQUAL_STRING("missing command-line argument: FILE",
        err_msg);
}

/* Test that a non-writable output file results in an error. */
TEST(parse_command_line_args, non_writable_output_file_gives_error)
{
    char *argv[] = {"ignore", "--output", "fake/file", "input_file"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");

    clear_err_msg();
    status = validate_command_line_args(&params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status, "validate status");
    TEST_ASSERT_EQUAL_STRING("failed to write to output file: "
        "fake/file", err_msg);
}

/* Test that the output file, if not specified, defaults to stdout. */
TEST(parse_command_line_args, output_file_defaults_to_stdout)
{
    char *argv[] = {"ignore", "test/data/input"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");

    clear_err_msg();
    status = validate_command_line_args(&params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "validate status");
    TEST_ASSERT_TRUE_MESSAGE(params.output_file == NULL,
        "output file defaults to stdout");
}

/* Test that an already existing and writable output file is
   accepted. */
TEST(parse_command_line_args, existing_writable_output_file)
{
    char *argv[] = {"ignore", "--output", "test/data/output.txt",
                    "test/data/input"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");

    clear_err_msg();
    status = validate_command_line_args(&params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "validate status");
}

/* Test that a non-existing input file gives an error. */
TEST(parse_command_line_args, non_existing_input_files_give_error)
{
    char *argv[] = {"ignore", "--output", "test/data/output.txt",
                    "foo"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");

    clear_err_msg();
    status = validate_command_line_args(&params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status, "validate status");
    TEST_ASSERT_EQUAL_STRING("failed to open file for reading: "
        "foo.iout", err_msg);
}

/* Test that a non-existent layout file causes an error. */
TEST(parse_command_line_args, non_existing_layout_file_gives_error)
{
    char *argv[] = {"ignore", "--output", "test/data/output.txt",
                    "test/data/data_only"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");

    clear_err_msg();
    status = validate_command_line_args(&params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status, "validate status");
    TEST_ASSERT_EQUAL_STRING("failed to open file for reading: "
        "test/data/data_only.iout", err_msg);
}

/* Test that a non-existent data file causes an error. */
TEST(parse_command_line_args, non_existing_data_file_gives_error)
{
    char *argv[] = {"ignore", "--output", "test/data/output.txt",
                    "test/data/layout_only"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");

    clear_err_msg();
    status = validate_command_line_args(&params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status, "validate status");
    TEST_ASSERT_EQUAL_STRING("failed to open file for reading: "
        "test/data/layout_only.out", err_msg);
}

/* Test that a non-existing but writable output file is gone after
   validate_command_line_args has tested it for writability. */
TEST(parse_command_line_args,
    non_existing_but_writable_output_file_gone_after_test)
{
    struct stat buf;
    char *output_file = "test/data/delete_me.txt";
    char *argv[] = {"ignore", "-o", output_file, "test/data/input"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");

    clear_err_msg();
    status = validate_command_line_args(&params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "validate status");

    clear_err_msg();
    errno = 0;
    status = stat(output_file, &buf);

    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, status, "stat return value");
    TEST_ASSERT_EQUAL_INT_MESSAGE(ENOENT, errno, "errno");
}

/* Test that specifying an empty string as the output file causes an
   error. */
TEST(parse_command_line_args, empty_output_filename_gives_error)
{
    char *argv[] = {"ignore", "--output", "", "test/data/input"};

    status = parse_command_line_args(NELEMS(argv), argv, &params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status, "parse status");

    clear_err_msg();
    status = validate_command_line_args(&params);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status, "validate status");
    TEST_ASSERT_EQUAL_STRING("output filename must not be empty",
        err_msg);
}

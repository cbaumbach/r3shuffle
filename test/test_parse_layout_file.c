#include "unity_fixture.h"
#include "parse_layout_file.h"
#include "err_msg.h"
#include <stddef.h>

#define NELEMS(x) (sizeof (x) / sizeof (x[0]))

/* In order to test the layout file parsing functions we will create
   our own temporary layout file using write_layout_file and read from
   it using parse_layout_file. */
static const char *file = "test/tmp/foo.iout";

static int status;
static struct Layout out, in;

static char *beta_labels[] = {"beta0", "beta1", "beta2"};
static char *se_labels[] = {"se0", "se1", "se2"};
static char *cov_labels[] = {"cov0_1", "cov0_2", "cov1_2"};
static char *snp_labels[] = {"snp0", "snp1", "snp2", "snp3", "snp4",
                             "snp5", "snp6", "snp7", "snp8", "snp9"};
static char *trait_labels[] = {"trait0", "trait1", "trait2", "trait3",
                               "trait4", "trait5", "trait6", "trait7"};

TEST_GROUP(parse_layout_file);

TEST_SETUP(parse_layout_file)
{
    /* Initialize the layout structure that will be written to the
       temporary layout file. */
    out.magic_number     = 6;
    out.bytes_per_double = sizeof(double);
    out.nvar             = NELEMS(beta_labels);
    out.nsnp             = NELEMS(snp_labels);
    out.ntrait           = NELEMS(trait_labels);
    out.snps_per_tile    = 4;
    out.traits_per_tile  = 3;
    out.max_char         = 10;
    out.ncov = ((NELEMS(beta_labels) - 1) * NELEMS(beta_labels)) / 2;
    out.beta_labels      = beta_labels;
    out.se_labels        = se_labels;
    out.cov_labels       = cov_labels;
    out.snp_labels       = snp_labels;
    out.trait_labels     = trait_labels;

    /* Initialize the layout structure that will receive the parsed
       content of the temporary layout file. */
    in.magic_number     = -1;
    in.bytes_per_double = -1;
    in.nvar             = -1;
    in.nsnp             = -1;
    in.ntrait           = -1;
    in.snps_per_tile    = -1;
    in.traits_per_tile  = -1;
    in.max_char         = -1;
    in.ncov             = -1;
    in.beta_labels      = NULL;
    in.se_labels        = NULL;
    in.cov_labels       = NULL;
    in.snp_labels       = NULL;
    in.trait_labels     = NULL;
}

TEST_TEAR_DOWN(parse_layout_file)
{
}

/* Test that when writing a layout file to disk and then reading it
   back in again we get the same layout. */
TEST(parse_layout_file, write_and_read_back_layout_file)
{
    int i;

    status = write_layout_file(file, &out);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status,
        "write_layout_file return value");

    clear_err_msg();
    status = parse_layout_file(file, &in);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status,
        "parse_layout_file return value");

    TEST_ASSERT_EQUAL_INT(out.magic_number,     in.magic_number);
    TEST_ASSERT_EQUAL_INT(out.bytes_per_double, in.bytes_per_double);
    TEST_ASSERT_EQUAL_INT(out.nvar,             in.nvar);
    TEST_ASSERT_EQUAL_INT(out.nsnp,             in.nsnp);
    TEST_ASSERT_EQUAL_INT(out.ntrait,           in.ntrait);
    TEST_ASSERT_EQUAL_INT(out.snps_per_tile,    in.snps_per_tile);
    TEST_ASSERT_EQUAL_INT(out.traits_per_tile,  in.traits_per_tile);
    TEST_ASSERT_EQUAL_INT(out.max_char,         in.max_char);
    TEST_ASSERT_EQUAL_INT(out.ncov,             in.ncov);

    TEST_ASSERT_TRUE(in.beta_labels != NULL);
    for (i = 0; i < out.nvar; i++)
        TEST_ASSERT_EQUAL_STRING_MESSAGE(out.beta_labels[i],
            in.beta_labels[i], "beta labels");

    TEST_ASSERT_TRUE(in.se_labels != NULL);
    for (i = 0; i < out.nvar; i++)
        TEST_ASSERT_EQUAL_STRING_MESSAGE(out.se_labels[i],
            in.se_labels[i], "se labels");

    TEST_ASSERT_TRUE(in.cov_labels != NULL);
    for (i = 0; i < ((out.nvar - 1) * out.nvar) / 2; i++)
        TEST_ASSERT_EQUAL_STRING_MESSAGE(out.cov_labels[i],
            in.cov_labels[i], "cov labels");

    TEST_ASSERT_TRUE(in.snp_labels != NULL);
    for (i = 0; i < out.nsnp; i++)
        TEST_ASSERT_EQUAL_STRING_MESSAGE(out.snp_labels[i],
            in.snp_labels[i], "snp labels");

    TEST_ASSERT_TRUE(in.trait_labels != NULL);
    for (i = 0; i < out.ntrait; i++)
        TEST_ASSERT_EQUAL_STRING_MESSAGE(out.trait_labels[i],
            in.trait_labels[i], "trait labels");
}

/* Test that a bad magic number causes an error. */
TEST(parse_layout_file, bad_magic_number)
{
    in = out;           /* pretend layout file was parsed correctly */
    in.magic_number = 99;       /* should be 6 */
    clear_err_msg();
    status = validate_layout(&in);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status,
        "validate_layout return value");
    TEST_ASSERT_EQUAL_STRING("bad magic number in layout file: "
        "expected 6, got 99", err_msg);
}

/* Test that a negative number of bytes per double causes an error. */
TEST(parse_layout_file, bad_bytes_per_double)
{
    in = out;           /* pretend layout file was parsed correctly */
    in.bytes_per_double = -1;   /* should be >0 */
    clear_err_msg();
    status = validate_layout(&in);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status,
        "validate_layout return value");
    TEST_ASSERT_EQUAL_STRING("bad number of bytes per double in "
        "layout file: expected >0, got -1", err_msg);
}

/* Test that a too small number of covariates causes an error. */
TEST(parse_layout_file, bad_number_of_covariates)
{
    in = out;           /* pretend layout file was parsed correctly */
    in.nvar = 1;        /* should be >=2 */
    clear_err_msg();
    status = validate_layout(&in);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status,
        "validate_layout return value");
    TEST_ASSERT_EQUAL_STRING("bad number of covariates in layout "
        "file: expected >=2, got 1", err_msg);
}

/* Test that write_layout_file throws an error upon receiving a
   non-positive number of snps in the layout structure. */
TEST(parse_layout_file,
    non_positive_number_of_snps_causes_error_during_write)
{
    out.nsnp = -1;              /* should be >0 */
    status = write_layout_file(file, &out);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status,
        "write_layout_file return value");
    TEST_ASSERT_EQUAL_STRING("bad number of snps in layout file: "
        "expected >0, got -1", err_msg);
}

/* Test that write_layout_file throws an error upon receiving a
   non-positive number of traits in the layout structure. */
TEST(parse_layout_file,
    non_positive_number_of_traits_causes_error_during_write)
{
    out.ntrait = -1;            /* should be >0 */
    status = write_layout_file(file, &out);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status,
        "write_layout_file return value");
    TEST_ASSERT_EQUAL_STRING("bad number of traits in layout file: "
        "expected >0, got -1", err_msg);
}

/* Test that write_layout_file throws an error upon receiving a
   non-positive number of covariates in the layout structure. */
TEST(parse_layout_file,
    non_positive_number_of_covariates_causes_error_during_write)
{
    out.nvar = -1;              /* should be >0 */
    status = write_layout_file(file, &out);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status,
        "write_layout_file return value");
    TEST_ASSERT_EQUAL_STRING("bad number of covariates in layout "
        "file: expected >0, got -1", err_msg);
}

/* Test that write_layout_file throws an error upon receiving a
   non-positive number of covariates in the layout structure. */
TEST(parse_layout_file,
    max_char_smaller_than_2_causes_error_during_write)
{
    out.max_char = -1;          /* should be >=2 */
    status = write_layout_file(file, &out);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status,
        "write_layout_file return value");
    TEST_ASSERT_EQUAL_STRING("bad max label length in layout file: "
        "expected >=2, got -1", err_msg);
}

/* Test that a non-positive number of snps in the layout file causes
   an error. */
TEST(parse_layout_file, bad_number_of_snps)
{
    in = out;           /* pretend layout file was parsed correctly */
    in.nsnp = -1;       /* should be >0 */
    clear_err_msg();
    status = validate_layout(&in);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status,
        "validate_layout return value");
    TEST_ASSERT_EQUAL_STRING("bad number of snps in layout file: "
        "expected >0, got -1", err_msg);
}

/* Test that a non-positive number of traits in the layout file causes
   an error. */
TEST(parse_layout_file, bad_number_of_traits)
{
    in = out;           /* pretend layout file was parsed correctly */
    in.ntrait = -1;     /* should be >0 */
    clear_err_msg();
    status = validate_layout(&in);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status,
        "validate_layout return value");
    TEST_ASSERT_EQUAL_STRING("bad number of traits in layout file: "
        "expected >0, got -1", err_msg);
}

/* Test that a non-positive number of snps per tile in the layout file
   causes an error. */
TEST(parse_layout_file, bad_number_of_snps_per_tile)
{
    in = out;           /* pretend layout file was parsed correctly */
    in.snps_per_tile = -1;      /* should be >0 */
    clear_err_msg();
    status = validate_layout(&in);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status,
        "validate_layout return value");
    TEST_ASSERT_EQUAL_STRING("bad number of snps per tile in layout "
        "file: expected >0, got -1", err_msg);
}

/* Test that a non-positive number of traits per tile in the layout
   file causes an error. */
TEST(parse_layout_file, bad_number_of_traits_per_tile)
{
    in = out;           /* pretend layout file was parsed correctly */
    in.traits_per_tile = -1;    /* should be >0 */
    clear_err_msg();
    status = validate_layout(&in);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status,
        "validate_layout return value");
    TEST_ASSERT_EQUAL_STRING("bad number of traits per tile in "
        "layout file: expected >0, got -1", err_msg);
}

/* Test that a maximum label length < 2 in layout file causes an
   error. */
TEST(parse_layout_file, bad_max_label_length)
{
    in = out;           /* pretend layout file was parsed correctly */
    in.max_char = -1;   /* should be >=2 */
    clear_err_msg();
    status = validate_layout(&in);

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status,
        "validate_layout return value");
    TEST_ASSERT_EQUAL_STRING("bad max label length in layout file: "
        "expected >=2, got -1", err_msg);
}

/* Test that we are able to map a user-supplied subset of column
   labels to the respective columns in the layout file. */
TEST(parse_layout_file, set_column_print_order_with_subset_of_columns)
{
    /* label: beta0 beta1 beta2 se0 se1 se2 cov0_1 cov0_2 cov1_2 */
    /* index: 0     1     2     3   4   5   6      7      8      */

    char *columns[] = {"se2", "cov0_2", "beta1"};
    int correct_ucp2acp[] = {5, 7, 1, -9};
    int ucp2acp[] = {-1, -1, -1, -9};
    int ncolumn = NELEMS(columns);
    int i;

    in = out;           /* pretend layout file was parsed correctly */
    clear_err_msg();
    status = set_column_print_order(columns, ncolumn, ucp2acp, &in);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status,
        "set_column_print_order return value");

    for (i = 0; i < ncolumn + 1; i++)
        TEST_ASSERT_EQUAL_INT(correct_ucp2acp[i], ucp2acp[i]);
}

/* Test that we are able to map user-supplied column labels that form
   a permutation of the actual column labels in the layout file to the
   actual labels in the layout file. */
TEST(parse_layout_file, set_permuted_column_print_order)
{
    /* label: beta0 beta1 beta2 se0 se1 se2 cov0_1 cov0_2 cov1_2 */
    /* index: 0     1     2     3   4   5   6      7      8      */

    char *columns[] = {"cov1_2", "beta2", "se1", "cov0_1", "se0",
                       "beta0", "se2", "cov0_2", "beta1"};
    int correct_ucp2acp[] = {8, 2, 4, 6, 3, 0, 5, 7, 1, -9};
    int ucp2acp[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -9};
    int ncolumn = NELEMS(columns);
    int i;

    in = out;           /* pretend layout file was parsed correctly */
    clear_err_msg();
    status = set_column_print_order(columns, ncolumn, ucp2acp, &in);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, status,
        "set_column_print_order return value");

    for (i = 0; i < ncolumn + 1; i++)
        TEST_ASSERT_EQUAL_INT(correct_ucp2acp[i], ucp2acp[i]);
}

/* Test that a user-supplied column label that does not correspond to
   any actual column label in the layout file causes an error. */
TEST(parse_layout_file, invalid_user_supplied_column_label)
{
    char *columns[] = {"cov1_2", "beta2", "foobar"};
    int ucp2acp[] = {-1, -1, -1, -9};
    int ncolumn = NELEMS(columns);

    in = out;           /* pretend layout file was parsed correctly */
    clear_err_msg();
    status = set_column_print_order(columns, ncolumn, ucp2acp, &in);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, status,
        "set_column_print_order return value");
    TEST_ASSERT_EQUAL_STRING("user-specified column doesn't exist: "
        "foobar", err_msg);
}

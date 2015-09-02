#include "unity_fixture.h"
#include "parse_data_file.h"
#include "parse_layout_file.h"

#define NELEMS(x) (sizeof (x) / sizeof (x[0]))

/* Test data:

   The layout of the (virtual) data file used in the below test cases
   is shown in parse_data_file.c Table 1.

   Both the snp_index array and the trait_index array are indexed by
   offset and contain the snp and the trait whose regression results
   are located at the given offset.

   The two-dimensional offsets array is indexed by snp (first
   dimension) and trait (second dimension) and contains the file
   offset at which the regression results for the given trait-snp pair
   can be found.*/
static struct Layout layout;

static int snp_index[] = {
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,  /* tile 1 */
    4, 5, 6, 7, 4, 5, 6, 7, 4, 5, 6, 7,  /* tile 2 */
    8, 9, 8, 9, 8, 9,                    /* tile 3 */
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,  /* tile 4 */
    4, 5, 6, 7, 4, 5, 6, 7, 4, 5, 6, 7,  /* tile 5 */
    8, 9, 8, 9, 8, 9,                    /* tile 6 */
    0, 1, 2, 3, 0, 1, 2, 3,              /* tile 7 */
    4, 5, 6, 7, 4, 5, 6, 7,              /* tile 8 */
    8, 9, 8, 9                           /* tile 9 */
};

static int trait_index[] = {
    0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,  /* tile 1 */
    0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,  /* tile 2 */
    0, 0, 1, 1, 2, 2,                    /* tile 3 */
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5,  /* tile 4 */
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5,  /* tile 5 */
    3, 3, 4, 4, 5, 5,                    /* tile 6 */
    6, 6, 6, 6, 7, 7, 7, 7,              /* tile 7 */
    6, 6, 6, 6, 7, 7, 7, 7,              /* tile 8 */
    6, 6, 7, 7                           /* tile 9 */
};

static unsigned long offsets[10][8] = {
    { 0,  4,  8, 30, 34, 38, 60, 64},  /* snp 0 */
    { 1,  5,  9, 31, 35, 39, 61, 65},  /* snp 1 */
    { 2,  6, 10, 32, 36, 40, 62, 66},  /* snp 2 */
    { 3,  7, 11, 33, 37, 41, 63, 67},  /* snp 3 */
    {12, 16, 20, 42, 46, 50, 68, 72},  /* snp 4 */
    {13, 17, 21, 43, 47, 51, 69, 73},  /* snp 5 */
    {14, 18, 22, 44, 48, 52, 70, 74},  /* snp 6 */
    {15, 19, 23, 45, 49, 53, 71, 75},  /* snp 7 */
    {24, 26, 28, 54, 56, 58, 76, 78},  /* snp 8 */
    {25, 27, 29, 55, 57, 59, 77, 79},  /* snp 9 */
};

TEST_GROUP(parse_data_file);

TEST_SETUP(parse_data_file)
{
    /* Initialize as much of the layout structure of the (virtual)
       layout file as needed for the test cases. */
    layout.magic_number     = -1;
    layout.bytes_per_double = -1;
    layout.nvar             = -1;
    layout.nsnp             = 10;
    layout.ntrait           = 8;
    layout.snps_per_tile    = 4;
    layout.traits_per_tile  = 3;
    layout.max_char         = -1;
    layout.ncov             = -1;
    layout.beta_labels      = NULL;
    layout.se_labels        = NULL;
    layout.cov_labels       = NULL;
    layout.snp_labels       = NULL;
    layout.trait_labels     = NULL;
}

TEST_TEAR_DOWN(parse_data_file)
{
}

/* Test whether offsets are correctly mapped to snps and traits. */
TEST(parse_data_file, offset2index)
{
    int i, n, snp, trait;

    n = layout.nsnp * layout.ntrait;

    for (i = 0; i < n; i++) {
        offset2index(i, &snp, &trait, &layout);
        TEST_ASSERT_EQUAL_INT_MESSAGE(snp_index[i], snp, "snp index");
        TEST_ASSERT_EQUAL_INT_MESSAGE(trait_index[i], trait,
            "trait index");
    }
}

/* Test whether trait-snp pairs are correctly mapped to offsets. */
TEST(parse_data_file, index2offset)
{
    int snp, trait;
    unsigned long offset;

    for (snp = 0; snp < layout.nsnp; snp++)
        for (trait = 0; trait < layout.ntrait; trait++) {
            index2offset(snp, trait, &offset, &layout);
            TEST_ASSERT_EQUAL_INT_MESSAGE(offsets[snp][trait],
                offset, "offset");
        }
}

#include "parse_data_file.h"
#include "parse_layout_file.h"
#include "err_msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* The binary data file contains the estimates that result from
   regressing ntrait traits on nsnp snps.  We can imagine all the
   possible regressions to be arranged into a matrix where every row
   corresponds to a trait and every column to a snp.  In the end,
   OmicABEL will have computed regression estimates corresponding to
   all cells in this matrix.  However, for computational reasons
   OmicABEL does not proceed line by line or row by row when running
   the regressions.  Instead, OmicABEL subdivides the matrix into
   so-called "tiles" and processes one tile at a time, from left to
   right and from top to bottom.

   Here is an example.  Suppose we have the following setup.

                          nsnps           10
                          ntraits          8
                          snps_per_tile    4
                          traits_per_tile  3

   Here is what the structure of the data file corresponding to this
   setup would look like.

                                snps

                   0  1  2  3    4  5  6  7    8  9
                +-------------+-------------+-------+
              0 |  0  1  2  3 | 12 13 14 15 | 24 25 |
              1 |  4  5  6  7 | 16 17 18 19 | 26 27 |
              2 |  8  9 10 11 | 20 21 22 23 | 28 29 |
                +-------------+-------------+-------+
     traits   3 | 30 31 32 33 | 42 43 44 45 | 54 55 |
              4 | 34 35 36 37 | 46 47 48 48 | 56 57 |
              5 | 38 39 40 41 | 50 51 52 53 | 58 59 |
                +-------------+-------------+-------+
              6 | 60 61 62 63 | 68 69 70 71 | 76 77 |
              7 | 64 65 66 67 | 72 73 74 75 | 78 79 |
                +-------------+-------------+-------+

       Figure 1. Layout of regression results in the data file.


   Note that we use 0-based numbering for both traits and snps.  This
   is the way C works and using the same convention here ensures that
   we do not have to add or subtract ones when switching between the
   code represention of the matrix and its visual representation given
   in this example.

   The numbers in the matrix represent the offset into the data file
   at which the regression estimates of the associated trait-snp pair
   can be found.  For example, the regression estimates obtained from
   regressing trait 7 on snp 6 can be found at offset 74.  The
   regression results for a given trait-snp pair consist of nvar
   betas, nvar standard errors, and ncov covariances.  The matrix in
   Figure 1 is subdivided into 9 tiles, where tiles are separated from
   each other with vertical and horizontal lines.  Tiles are ordered
   and the order is defined by the offsets in the cells.  A tile
   containing higher offsets ranks higher than a tile with lower
   offsets.  The regression estimates of the first tile can be found
   at offsets 0 through 11.  The regression estimates of the second
   tile can be found at offsets 12 through 23, and so on.  If we
   replace the tiles' contents by their respective rank, we get the
   matrix depicted in Figure 2.

                                snps

                            +---+---+---+
                            | 1 | 2 | 3 |
                            +---+---+---+
                   traits   | 4 | 5 | 6 |
                            +---+---+---+
                            | 7 | 8 | 9 |
                            +---+---+---+

            Figure 2. Order of tiles within the data file.


   Thus the order of tiles corresponds to their position in the data
   file.  First come the regression estimates of tile 1, then of tile
   2, and so on.  We will use the term "tile row" to refer to a row of
   tiles.  The above matrix has three tile rows consisting of tiles 1
   through 3, tiles 4 through 6, and tiles 7 through 9, respectively.
   We use the term "tile column" to refer to a column of tiles.  The
   above matrix has three tile columns consisting of tiles 1, 4, 7,
   tiles 2, 5, 8, and tiles 3, 6, 9, respectively.  With every tile we
   can associate the set of traits corresponding to the rows occupied
   by the tile and the set of snps corresponding to the columns that
   the tile occupies.  For example, tile 5 corresponds to traits 3 to
   5 and snps 4 to 7.  A tile contains the regression estimates
   resulting from regressing every element of its set of traits on
   every element of its set of snps.  Every tile, except maybe those
   at the lower and right margin, contains traits_per_tile *
   snps_per_tile regression estimates.  If snps_per_tile does not
   divide nsnp evenly, the tiles at the right margin will represent
   nsnp modulo snps_per_tile instead of snps_per_tile snps.
   Similarly, if traits_per_tile does not divide ntrait evenly, the
   tiles at the bottom margin will represent ntrait modulo
   traits_per_tile instead of traits_per_tile snps. */

/* When converting the binary data file into a human-readable plain
   text file, we have a choice as to the order in which we process the
   data file.  One way would be to go through the data file in
   sequential order starting at offset 0.  Due to the layout of the
   data file this is the same as proceeding by tile starting at tile
   0.  If we want to include the regression estimates at a given
   offset in the output, we would have to map the offset to a trait
   index and a snp index.  For example, being at offset 47, we look up
   the row and column and find that the offset holds the regression
   estimates for trait 4 and snp 5.

   Here is how we would proceed to map offset 47 to the corresponding
   snp and trait indexes.

   +---+---+---+
   | 1 | 2 | 3 |                                 +-------------+
   +---+---+---+  1   +---+---+---+  2   +---+   | 42 43 44 45 |
   | 4 | 5 | 6 | ---> | 4 | 5 | 6 | ---> | 5 | = | 46 47 48 48 |
   +---+---+---+      +---+---+---+      +---+   | 50 51 52 53 |
   | 7 | 8 | 9 |                                 +-------------+
   +---+---+---+

   +-------------+
   | 42 43 44 45 |  3   +-------------+  4   +----+
   | 46 47 48 48 | ---> | 46 47 48 48 | ---> | 47 |
   | 50 51 52 53 |      +-------------+      +----+
   +-------------+

                     1. Find tile row.
                     2. Find tile column.
                     3. Find row within tile.
                     4. Find column within tile.

          Figure 3. Mapping offset 47 to trait 4 and snp 5. */


/* Map an offset to a snp index and a trait index. */
void offset2index(unsigned long offset, int *snp, int *trait,
    struct Layout *layout)
{
    int elts_per_tile_row, elts_per_tile, snps_in_this_tile;
    int tile_row, tile_col, row_within_tile, col_within_tile;
    int snps_per_tile, traits_per_tile, nsnp, ntrait;
    unsigned long x;

    snps_per_tile = layout->snps_per_tile;
    traits_per_tile = layout->traits_per_tile;
    nsnp = layout->nsnp;
    ntrait = layout->ntrait;

    x = offset;

    /* Find tile row:

       In order to find the tile row into which the offset points we
       have to count how many times we can fit the cells contained in
       a tile row into the offset.  We know that a typical tile row
       contains nsnp * traits_per_tile cells.  (It is true that the
       bottom tile row can contain fewer cells.  But this is
       irrelevant for finding the tile row.)  After finding the tile
       row we update the offset such that it becomes a valid offset
       into the tile row. */
    elts_per_tile_row = nsnp * traits_per_tile;
    tile_row = x / elts_per_tile_row;
    x %= elts_per_tile_row;

    /* Find tile column:

       With the tile row found and the offset updated to point to one
       of its cells we now have to find the tile column.  We need to
       count how many times we can fit the number of cells in a tile
       into the offset.  A typical, full-size tile contains
       snps_per_tile * traits_per_tile cells.  However, if the total
       number of traits in the current and all following tile rows
       (ntrait - tile_row * traits_per_tile) is smaller than the
       number of traits in a typical full-size tile, we are in an
       undersized bottom tile row.  Since this effects all tiles in
       the row we cannot just ignore it as we ignored the undersized
       bottom tile row when searching for the tile row.  After having
       found the tile column we need to update the offset such that it
       becomes a valid offset into the tile determine by tile row and
       tile column. */
    if (traits_per_tile <= ntrait - tile_row * traits_per_tile)
        elts_per_tile = snps_per_tile * traits_per_tile;
    else
        elts_per_tile = snps_per_tile
            * (ntrait - tile_row * traits_per_tile);
    tile_col = x / elts_per_tile;
    x %= elts_per_tile;

    /* Find row within tile:

       To find the row within the tile we have to count how many times
       we can fit the number of cells contained in a row of a tile
       into the offset.  A typical, full-size row contains
       snps_per_tile.  However, if the total number of snps in the
       current and all following tile columns (nsnp - tile_col *
       snps_per_tile) is smaller than the number of snps in a typical
       full-size row, we are in an undersized rightmost tile column.
       After having found the row within the tile we update the offset
       such that it becomes a valid offset into the row. */
    if (snps_per_tile <= nsnp - tile_col * snps_per_tile)
        snps_in_this_tile = snps_per_tile;
    else
        snps_in_this_tile = nsnp - tile_col * snps_per_tile;
    row_within_tile = x / snps_in_this_tile;
    x %= snps_in_this_tile;

    /* Find column within tile:

       The updated the offset already points to the column within the
       tile. */
    col_within_tile = x;

    *trait = tile_row * traits_per_tile + row_within_tile;
    *snp   = tile_col * snps_per_tile   + col_within_tile;
}

/* Another way to process the data file, which is more efficient if
   only a small number of trait-snp pairs is to be extracted, is to
   jump directly to the offsets of specific trait-snp pairs.  The
   index2offset function takes a snp and a trait and returns the
   offset in the data file at which the corresponding regression
   results can be found.  The computation is very similar to the one
   used in offset2index and Figure 3 is helpful in following the
   steps. */

/* Map a snp index and a trait index to an offset. */
void index2offset(int snp, int trait, unsigned long *offset,
    struct Layout *layout)
{
    int tile_row, tile_col, row_within_tile, col_within_tile;
    int snps_per_tile, traits_per_tile, nsnp, ntrait;
    unsigned long x;

    snps_per_tile = layout->snps_per_tile;
    traits_per_tile = layout->traits_per_tile;
    nsnp = layout->nsnp;
    ntrait = layout->ntrait;

    /* Find tile row and tile column:

       We find the tile row by counting how many tile rows, each
       containing traits_per_tile traits, we can fill before we reach
       trait trait.  Similarly, we find the tile column by counting
       how many tile columns, each containing snps_per_tile snps, we
       can fill before we reach snp snp. */
    tile_row = trait / traits_per_tile;
    tile_col = snp / snps_per_tile;

    /* Find row within tile and column within tile:

       The row within the tile is what's left from trait after filling
       tile_row tile rows.  Similarly, the column within the tile is
       what's left from snp after filling tile_col tile columns. */
    row_within_tile = trait % traits_per_tile;
    col_within_tile = snp % snps_per_tile;

    x = 0;

    /* Advance offset until just after tile row tile_row. */
    x += tile_row * nsnp * traits_per_tile;

    /* Advance offset until just after tile column tile_col.  For an
       explanation of the if-condition see offset2index. */
    if (traits_per_tile <= ntrait - tile_row * traits_per_tile)
        x += tile_col * snps_per_tile * traits_per_tile;
    else
        x += tile_col * snps_per_tile
            * (ntrait - tile_row * traits_per_tile);

    /* Advance offset until just after row row_within_tile.  For an
       explanation of the if-condition see offset2index. */
    if (snps_per_tile <= nsnp - tile_col * snps_per_tile)
        x += row_within_tile * snps_per_tile;
    else
        x += row_within_tile * (nsnp - tile_col * snps_per_tile);

    /* Advance offset until just after column col_within_tile. */
    x += col_within_tile;

    *offset = x;
}

int parse_data_file(struct Params *params, struct Layout *layout)
{
    FILE *ifp, *ofp;
    int nrecord;      /* number of result records in data file */
    int nrec;         /* number of records read so far */
    int ncolumn;      /* number of columns in regression results */
    size_t nbytes;    /* number of bytes used by regression results */
    char *buf;        /* buffer to hold regression result bytes */
    double *v;        /* buffer to hold actual regression results */
    int snp;          /* index of snp in current trait-snp pair */
    int trait;        /* index of trait in current trait-snp pair */
    int i;

    if ((ifp = fopen(params->data_file, "rb")) == NULL) {
        set_err_msg("failed to open file for reading: %s",
            params->data_file);
        goto RETURN_ZERO;
    }

    if (params->output_file == NULL)
        ofp = stdout;
    else if ((ofp = fopen(params->output_file, "wb")) == NULL) {
        set_err_msg("failed to open file for writing: %s",
            params->output_file);
        goto CLOSE_DATA_FILE;
    }

    /* Allocate a buffer large enough to hold the bytes containing the
       regression results of a single trait-snp pair.  There are nvar
       betas, nvar standard errors, and ncov covariances.  Each value
       represents a double of bytes_per_double bytes. */
    ncolumn = layout->nvar + layout->nvar + layout->ncov;
    nbytes = ncolumn * layout->bytes_per_double;
    if ((buf = (char *) malloc(nbytes)) == NULL) {
        set_err_msg("failed to allocate %lu bytes",
            (unsigned long) nbytes);
        goto CLOSE_OUTPUT_FILE;
    }

    /* Print header. */
    fprintf(ofp, "trait snp");
    if (params->ncolumn)
        for (i = 0; i < params->ncolumn; i++)
            fprintf(ofp, " %s", params->columns[i]);
    else {
        for (i = 0; i < layout->nvar; i++)
            fprintf(ofp, " %s", layout->beta_labels[i]);
        for (i = 0; i < layout->nvar; i++)
            fprintf(ofp, " %s", layout->se_labels[i]);
        for (i = 0; i < layout->ncov; i++)
            fprintf(ofp, " %s", layout->cov_labels[i]);
    }
    fprintf(ofp, "\n");

    nrecord = layout->nsnp * layout->ntrait;
    nrec = 0;
    while (nrec < nrecord) {
        assert(1 == fread(buf, nbytes, 1, ifp));
        v = (double *) buf;
        offset2index(nrec, &snp, &trait, layout);
        fprintf(ofp, "%s %s", layout->trait_labels[trait],
            layout->snp_labels[snp]);
        if (params->ncolumn)
            for (i = 0; i < params->ncolumn; i++)
                fprintf(ofp, " %.*g", params->ndigit,
                    v[params->ucp2acp[i]]);
        else
            for (i = 0; i < ncolumn; i++)
                fprintf(ofp, " %.*g", params->ndigit, v[i]);
        fprintf(ofp, "\n");
        ++nrec;
    }

    free(buf);

    if (params->output_file != NULL  &&  fclose(ofp)) {
        set_err_msg("failed to close file: %s",
            params->output_file);
        goto CLOSE_DATA_FILE;
    }

    if (fclose(ifp)) {
        set_err_msg("failed to close file: %s",
            params->data_file);
        goto RETURN_ZERO;
    }

    return 1;

    /* Don't use set_err_msg from here on: if we got here, it means
       there already was a problem and set_err_msg was used.  Using it
       again would overwrite the error message which states the
       initial problem and thus the actual reason behind the 0 return
       value. */
CLOSE_OUTPUT_FILE:
    if (params->output_file != NULL)
        fclose(ofp);
CLOSE_DATA_FILE:
    fclose(ifp);
RETURN_ZERO:
    return 0;
}

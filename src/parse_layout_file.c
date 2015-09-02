#include "parse_layout_file.h"
#include "err_msg.h"
#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define NELEMS(x) (sizeof (x) / sizeof (x[0]))

int parse_layout_file(const char *file, struct Layout *layout)
{
    int a[8], i;
    FILE *fp;
    int nlabel;                 /* number of labels in layout file */
    size_t n;                   /* number of bytes used by labels */
    char *buf, *s, **t;

    if ((fp = fopen(file, "rb")) == NULL) {
        set_err_msg("failed to open layout file for reading: %s", file);
        return 0;
    }

    assert(fread(a, sizeof(int), NELEMS(a), fp) == NELEMS(a));
    if (feof(fp)) {
        set_err_msg("unexpectedly reached end of layout file: %s", file);
        return 0;
    }
    if (ferror(fp)) {
        set_err_msg("error while reading layout file: %s", file);
        return 0;
    }

    layout->magic_number     = a[0];
    layout->bytes_per_double = a[1];
    layout->nvar             = a[2];
    layout->nsnp             = a[3];
    layout->ntrait           = a[4];
    layout->snps_per_tile    = a[5];
    layout->traits_per_tile  = a[6];
    layout->max_char         = a[7];

    /* The number of covariances between the covariates corresponds to
       the number of elements in the lower covariance matrix (not
       counting the diagonal elements).  Since we have nvar covariates
       the lower covariance matrix contains 1 + 2 + ... + nvar - 1 =
       (nvar - 1) * nvar / 2 elements. */
    layout->ncov = ((layout->nvar - 1) * layout->nvar) / 2;

    /* Read labels from layout file.

       We have nvar beta labels, nvar standard error labels, ncov
       covariances, nsnp snp labels, and ntrait trait labels.  Each
       label occupies max_char bytes. */
    nlabel = layout->nvar + layout->nvar + layout->ncov + layout->nsnp
        + layout->ntrait;
    n = nlabel * layout->max_char;
    if ((buf = (char *) malloc(n)) == NULL) {
        set_err_msg("failed to allocate %lu bytes",
            (unsigned long) n);
        return 0;
    }
    assert(fread(buf, n, 1, fp) == 1);

    /* Since all labels are contained in buf, the label members of the
       layout struct will be nothing but pointers into buf.  Before we
       can set those pointers we have to allocate space for them. */
    if ((t = (char **) malloc(nlabel * sizeof(char *))) == NULL) {
        set_err_msg("failed to allocate %lu bytes for labels",
            (unsigned long) n);
        return 0;
    }
    layout->beta_labels  = t;
    layout->se_labels    = layout->beta_labels + layout->nvar;
    layout->cov_labels   = layout->se_labels   + layout->nvar;
    layout->snp_labels   = layout->cov_labels  + layout->ncov;
    layout->trait_labels = layout->snp_labels  + layout->nsnp;

    /* The first nvar strings, each of length max_char, correspond to
       the beta coefficient labels.  Then come nvar standard error
       labels, followed by ncov covariance labels, nsnp snp labels,
       and finally ntrait trait labels.  We move a pointer to char
       over buf in jumps of max_char bytes.  Hence, the pointer will
       always point to the address of the first character of some
       label.  The label addresses and thus the labels themselves are
       stored in the corresponding label pointer bucket. */
    s = buf;
    for (i = 0; i < layout->nvar; i++, s += layout->max_char)
        layout->beta_labels[i] = s;
    for (i = 0; i < layout->nvar; i++, s += layout->max_char)
        layout->se_labels[i] = s;
    for (i = 0; i < layout->ncov; i++, s += layout->max_char)
        layout->cov_labels[i] = s;
    for (i = 0; i < layout->nsnp; i++, s += layout->max_char)
        layout->snp_labels[i] = s;
    for (i = 0; i < layout->ntrait; i++, s += layout->max_char)
        layout->trait_labels[i] = s;

    if (fclose(fp)) {
        set_err_msg("failed to close layout file after reading: %s",
            file);
        return 0;
    }

    return 1;
}

int validate_layout(struct Layout *layout)
{
    if (layout->magic_number != 6) {
        set_err_msg("bad magic number in layout file: expected 6, got %d",
            layout->magic_number);
        return 0;
    }

    if (layout->bytes_per_double <= 0) {
        set_err_msg("bad number of bytes per double in layout file: "
            "expected >0, got %d", layout->bytes_per_double);
        return 0;
    }

    /* We have at least 2 covariates: the intercept and the snp. */
    if (layout->nvar < 2) {
        set_err_msg("bad number of covariates in layout file: "
            "expected >=2, got %d", layout->nvar);
        return 0;
    }

    if (layout->nsnp <= 0) {
        set_err_msg("bad number of snps in layout file: "
            "expected >0, got %d", layout->nsnp);
        return 0;
    }

    if (layout->ntrait <= 0) {
        set_err_msg("bad number of traits in layout file: "
            "expected >0, got %d", layout->ntrait);
        return 0;
    }

    if (layout->snps_per_tile <= 0) {
        set_err_msg("bad number of snps per tile in layout file: "
            "expected >0, got %d", layout->snps_per_tile);
        return 0;
    }

    if (layout->traits_per_tile <= 0) {
        set_err_msg("bad number of traits per tile in layout file: "
            "expected >0, got %d", layout->traits_per_tile);
        return 0;
    }

    /* The smallest possible label consists of a single character plus
       the terminating NUL character. */
    if (layout->max_char < 2) {
        set_err_msg("bad max label length in layout file: "
            "expected >=2, got %d", layout->max_char);
        return 0;
    }

    return 1;
}

int write_layout_file(const char *file, struct Layout *layout)
{
    int a[8], i;
    FILE *fp;
    char *buf, *s;
    int nlabel;                 /* number of labels in layout file */
    size_t n;                   /* number of bytes used by labels */

    if ((fp = fopen(file, "wb")) == NULL) {
        set_err_msg("failed to open layout file for writing: %s", file);
        return 0;
    }

    if (layout->max_char < 2) {
        set_err_msg("bad max label length in layout file: "
            "expected >=2, got %d", layout->max_char);
        return 0;
    }

    if (layout->nsnp <= 0) {
        set_err_msg("bad number of snps in layout file: "
            "expected >0, got %d", layout->nsnp);
        return 0;
    }

    if (layout->ntrait <= 0) {
        set_err_msg("bad number of traits in layout file: "
            "expected >0, got %d", layout->ntrait);
        return 0;
    }

    if (layout->nvar <= 0) {
        set_err_msg("bad number of covariates in layout file: "
            "expected >0, got %d", layout->nvar);
        return 0;
    }

    a[0] = layout->magic_number;
    a[1] = layout->bytes_per_double;
    a[2] = layout->nvar;
    a[3] = layout->nsnp;
    a[4] = layout->ntrait;
    a[5] = layout->snps_per_tile;
    a[6] = layout->traits_per_tile;
    a[7] = layout->max_char;

    assert(fwrite(a, sizeof(int), NELEMS(a), fp) == NELEMS(a));
    if (ferror(fp)) {
        set_err_msg("error while writing layout file: %s", file);
        return 0;
    }

    /* Write labels from layout file.

       We have nvar beta labels, nvar standard error labels, ncov
       covariances, nsnp snp labels, and ntrait trait labels.  Each
       label occupies max_char bytes. */
    nlabel = layout->nvar + layout->nvar + layout->ncov + layout->nsnp
        + layout->ntrait;
    n = nlabel * layout->max_char;

    if ((buf = (char *) malloc(n * sizeof(char))) == NULL) {
        set_err_msg("failed to allocate %lu bytes", (unsigned long) n);
        return 0;
    }
    /* Since we are going to use [[buf]] for storing NUL terminated
       strings we initialize it with NUL characters. */
    for (i = 0; (unsigned) i < n; i++)
        buf[i] = '\0';

    /* Copy labels into buf.  To be on the safe side we explicitly
       terminate every label with a NUL character. */
    s = buf;
    for (i = 0; i < layout->nvar; i++, s += layout->max_char) {
        strncpy(s, layout->beta_labels[i], layout->max_char);
        s[layout->max_char - 1] = '\0';
    }
    for (i = 0; i < layout->nvar; i++, s += layout->max_char) {
        strncpy(s, layout->se_labels[i], layout->max_char);
        s[layout->max_char - 1] = '\0';
    }
    for (i = 0; i < layout->ncov; i++, s += layout->max_char) {
        strncpy(s, layout->cov_labels[i], layout->max_char);
        s[layout->max_char - 1] = '\0';
    }
    for (i = 0; i < layout->nsnp; i++, s += layout->max_char) {
        strncpy(s, layout->snp_labels[i], layout->max_char);
        s[layout->max_char - 1] = '\0';
    }
    for (i = 0; i < layout->ntrait; i++, s += layout->max_char) {
        strncpy(s, layout->trait_labels[i], layout->max_char);
        s[layout->max_char - 1] = '\0';
    }

    /* Write labels to layout file. */
    assert(fwrite(buf, n, 1, fp) == 1);
    free(buf);
    buf = NULL;

    if (fclose(fp)) {
        set_err_msg("failed to close layout file: %s", file);
        return 0;
    }

    return 1;
}

/* Every entry with regression results in the data file is ordered in
   the same way.  There are nvar beta column, then nvar standard error
   columns, and eventually ncov covariance columns.  By using the
   --column option on the command-line the user not only decides which
   of these columns will be included in the output but also in which
   order they will be included.  We will map the position of a
   user-supplied column label (user column position, ucp) to the index
   of the corresponding column among the above regression result
   columns (actual column position, acp). */
int set_column_print_order(char **columns, int ncolumn, int *ucp2acp,
    struct Layout *layout)
{
    int i, j, n, found;
    char *s;

    n = layout->nvar + layout->nvar + layout->ncov;

    for (i = 0; i < ncolumn; i++) {

        found = 0;
        s = columns[i];

        /* Search user column label among beta labels. */
        for (j = 0; j < layout->nvar && !found; j++)
            if (!strncmp(s, layout->beta_labels[j], layout->max_char)) {
                ucp2acp[i] = j;
                found = 1;
            }

        /* Search user column label among standard error labels.  */
        for (j = 0; j < layout->nvar && !found; j++)
            if (!strncmp(s, layout->se_labels[j], layout->max_char)) {
                ucp2acp[i] = layout->nvar + j;
                found = 1;
            }

        /* Search user column label among covariance labels. */
        for (j = 0; j < layout->ncov && !found; j++)
            if (!strncmp(s, layout->cov_labels[j], layout->max_char)) {
                ucp2acp[i] = layout->nvar + layout->nvar + j;
                found = 1;
            }

        if (!found) {
            set_err_msg("user-specified column doesn't exist: %s", s);
            return 0;
        }
    }
    return 1;
}

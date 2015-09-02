#ifndef PARSE_LAYOUT_FILE_H
#define PARSE_LAYOUT_FILE_H

struct Layout {
    int magic_number;      /* magic number */
    int bytes_per_double;  /* number of bytes in a double */
    int nvar; /* number of covariates (including intercept and snp) */
    int nsnp;              /* number of snps */
    int ntrait;            /* number of traits */
    int snps_per_tile;     /* number of snps per tile (see below) */
    int traits_per_tile;   /* number of traits per tile (see below) */
    int max_char;          /* number of characters per label */
    int ncov;              /* number of covariances */
    char **beta_labels;    /* labels for beta columns */
    char **se_labels;      /* labels for standard error columns */
    char **cov_labels;     /* labels for covariance columns */
    char **snp_labels;     /* snp labels */
    char **trait_labels;   /* trait labels */
};

int parse_layout_file(const char *file, struct Layout *layout);

int validate_layout(struct Layout *layout);

int write_layout_file(const char *file, struct Layout *layout);

int set_column_print_order(char **columns, int ncolumn, int *ucp2acp,
    struct Layout *layout);

#endif  /* PARSE_LAYOUT_FILE_H */

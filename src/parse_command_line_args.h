#ifndef PARSE_COMMAND_LINE_ARGS_H
#define PARSE_COMMAND_LINE_ARGS_H

struct Params {
    int ncolumn;                /* number of selected columns */
    char **columns;             /* labels of selected columns */
    int *ucp2acp; /* user column position -> actual column position */
    int ndigit;                 /* number of sig. digits in output */
    int help;                   /* Display help message? */
    int print_columns;          /* Print available columns? */
    char *output_file;          /* path to output file */
    char *layout_file;          /* path to layout file */
    char *data_file;            /* path to data file */
    char fmt[10];               /* print format string */
};

void initialize_parameters(struct Params *params);

int parse_command_line_args(int argc, char *argv[],
    struct Params *params);

int validate_command_line_args(struct Params *params);

#endif  /* PARSE_COMMAND_LINE_ARGS_H */

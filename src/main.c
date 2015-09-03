#include "parse_command_line_args.h"
#include "parse_layout_file.h"
#include "parse_data_file.h"
#include "err_msg.h"
#include <stdlib.h>

void usage(void);

int main(int argc, char **argv)
{
    struct Params params;
    struct Layout layout;

    initialize_parameters(&params);
    if (!parse_command_line_args(argc, argv, &params))
        goto ERROR;

    if (params.help) {
        usage();
        goto SUCCESS;
    }

    if (!validate_command_line_args(&params))
        goto ERROR;

    if (!parse_layout_file(params.layout_file, &layout))
        goto ERROR;

    if  (!validate_layout(&layout))
        goto ERROR;

    if (params.print_columns) {
        print_columns(&layout);
        goto SUCCESS;
    }

    if (!set_column_print_order(&params, &layout))
        goto ERROR;

    if (!parse_data_file(&params, &layout))
        goto ERROR;

SUCCESS:
    exit(EXIT_SUCCESS);

ERROR:
    pr_err_msg();
    exit(EXIT_FAILURE);
}

void usage(void)
{
    fprintf(stderr,
        "NAME\n"
        "       r3shuffle - convert OmicABEL's binary output to plain text\n"
        "\n"
        "SYNOPSIS\n"
        "       r3shuffle [OPTION]... FILE\n"
        "\n"
        "DESCRIPTION\n"
        "       Convert OmicABEL's binary output files FILE.iout and\n"
        "       FILE.out into a single plain text file.\n"
        "\n"
        "       Mandatory arguments to long options are mandatory for short\n"
        "       options too.\n"
        "\n"
        "       -c, --column=LABEL\n"
        "              include column LABEL in output\n"
        "\n"
        "       -d, --digits=K\n"
        "              use K significant digits in output (default: 8)\n"
        "\n"
        "       -h, --help\n"
        "              display this help message\n"
        "\n"
        "       -o, --output=OUTFILE\n"
        "              name of output file (default: stdout)\n"
        "\n"
        "       --print-columns\n"
        "              write available output variables to --output\n");
}

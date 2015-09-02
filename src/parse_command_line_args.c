#include "parse_command_line_args.h"
#include "err_msg.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>

#define NELEMS(x) (sizeof (x) / sizeof (x[0]))

void initialize_parameters(struct Params *params)
{
    int i;

    params->ncolumn = 0;
    params->columns = NULL;
    params->ucp2acp = NULL;
    params->ndigit  = 8;
    params->help    = 0;
    params->print_columns = 0;
    params->output_file = NULL;
    params->layout_file = NULL;
    params->data_file   = NULL;
    for (i = 0; (unsigned)i < NELEMS(params->fmt); i++)
        params->fmt[i] = '\0';
}

int parse_command_line_args(int argc, char *argv[],
    struct Params *params)
{
    int c, *p, i;
    long v;
    char *s, **t;
    size_t n;

    while (1) {

        static struct option long_options[] = {
            {"column",        required_argument, 0, 'c'},
            {"digits",        required_argument, 0, 'd'},
            {"help",          no_argument,       0, 'h'},
            {"output",        required_argument, 0, 'o'},
            {"print-columns", no_argument,       0, 'p'},
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, ":c:d:ho:", long_options, NULL);
        if (c == -1)
            break;

        switch (c) {

        case 'c':
            /* Every time we find a new column we extend our storage
               of pointers to char by one and store the label of the
               given column in the freshly allocated cell. */
            n = (params->ncolumn + 1) * sizeof(char *);
            if ((t = (char **) realloc(params->columns, n)) == NULL) {
                set_err_msg("failed to reallocate %lu bytes for "
                    "user-supplied column labels", (unsigned long) n);
                return 0;
            }
            params->columns = t;
            params->columns[params->ncolumn++] = optarg;
            break;

        case 'd':
            /* If the call to strtol results in underflow or overflow,
               errno is set to ERANGE.  It can also happen that strtol
               doesn't find a single valid character in its first
               argument.  In that case strtol will return 0 and maybe
               set errno.  We can detect this situation for certain by
               checking whether, after the call to strtol, the pointer
               passed as the second argument points to the same
               address as the pointer passed as the first argument. */
            errno = 0;
            v = strtol(optarg, &s, 10);
            if (errno  ||  s == optarg) {
                set_err_msg("failed to convert --digits to integer: "
                    "%s", optarg);
                return 0;
            }
            params->ndigit = v;
            break;

        case 'h':
            params->help = 1;
            break;

        case 'o':
            params->output_file = optarg;
            break;

        case 'p':
            params->print_columns = 1;
            break;

        case ':':
            set_err_msg("missing argument: %s", argv[optind - 1]);
            return 0;

        default:
            set_err_msg("unknown option: %s", argv[optind - 1]);
            return 0;
        }
    }

    /* The user-supplied column labels determine the order in which
       columns will be written to the output file.  Once we know the
       actual column labels as defined in the layout file, we have to
       associate every user-supplied column label with its index in
       the list of labels in the layout file.  Here we only allocate a
       buffer to hold the column indexes that we will compute later.
       We allocate space for an additional integer that will be
       initialized to -9 and will be used for testing. */
    n = (params->ncolumn + 1) * sizeof(int);
    if ((p = (int *) malloc(n)) == NULL) {
        set_err_msg("failed to allocate %lu bytes",
            (unsigned long) n);
        return 0;
    }
    for (i = 0; i < params->ncolumn; i++)
        p[i] = -1;
    p[params->ncolumn] = -9;
    params->ucp2acp = p;

    /* Get path to layout and data files. */
    if (optind < argc) {
        char *s;
        size_t len = strlen(argv[optind]);
        size_t nchar;
        const char data_extension[] = ".out";
        const char layout_extension[] = ".iout";

        nchar = len + sizeof data_extension;  /* includes NUL byte */
        assert((s = (char *) malloc(nchar)) != NULL);
        assert(nchar - 1 == (size_t) sprintf(s, "%s%s", argv[optind],
                data_extension));
        params->data_file = s;

        nchar = len + sizeof layout_extension; /* includes NUL byte */
        assert((s = (char *) malloc(nchar)) != NULL);
        assert(nchar - 1 == (size_t) sprintf(s, "%s%s", argv[optind],
                layout_extension));
        params->layout_file = s;
    }

    return 1;
}

int validate_command_line_args(struct Params *params)
{
    FILE *fp;
    const char *file;
    int output_file_exists;
    struct stat buf;
    int status;

    /* We require the number of significant digits be non-negative and
       <= 99.  This way we know that the width specification in the
       format will never occupy more than 2 characters. */
    if (params->ndigit > 99) {
        set_err_msg("argument to --digits must be <100");
        return 0;
    }
    if (params->ndigit < 0) {
        set_err_msg("argument to --digits must be >=0");
        return 0;
    }
    sprintf(params->fmt, "%%.%df", params->ndigit);

    /* Check that output file is writable. */
    if ((file = params->output_file) != NULL) {

        if (file[0] == '\0') {
            set_err_msg("output filename must not be empty");
            return 0;
        }

        /* Check if output file exists already.

           If we are able to retrieve information about the output
           file via stat, we can be sure that it exists.  In case stat
           fails to retrieve information about the file, we
           distinguish two cases.  If stat sets errno to ENOENT, the
           man page stat says that "A component of path does not
           exist, or path is an empty string."  Since we have already
           checked for an empty path, we know that some component of
           the path does not exist.  But this means that the output
           file doesn't exist.  If we find errno to have a value other
           than ENOENT, we will return an error and leave further
           investigation of the problem to the user. */
        errno = 0;
        status = stat(file, &buf);
        if (status == 0)
            output_file_exists = 1;
        else {
            switch(errno) {

            case ENOENT:
                output_file_exists = 0;
                break;

            case ELOOP:
                set_err_msg("too many symbolic links while traversing path: %s", file);
                return 0;

            case ENAMETOOLONG:
                set_err_msg("file name too long: %s", file);
                return 0;

            case ENOMEM:
                set_err_msg("out of memory while trying to stat(2): %s", file);
                return 0;

            case ENOTDIR:
                set_err_msg("some component of path prefix is not a directory: %s", file);
                return 0;

            case EOVERFLOW:
                set_err_msg("overflow while trying to stat(2): %s", file);
                return 0;

            default:
                set_err_msg("failed to stat(2) output file: %s", file);
                return 0;
            }
        }

        /* Check that output file is writable.

           Opening a file in write mode has the inconvenient
           side-effect of truncating the file to zero length.
           However, we don't want to clobber an existing file
           prematurely.  Maybe something causes the program to abort
           before we even start writing to the output file.  In such a
           case, we would be left with nothing, neither the old nor
           the new output file.  To avoid this scenario we will try to
           open the output file in append mode.  This creates the file
           if it doesn't already exist but it doesn't truncate an
           existing file.  If we created the file during the append,
           we remove the file once the append succeeded. */
        if ((fp = fopen(file, "ab")) == NULL) {
            set_err_msg("failed to write to output file: %s", file);
            return 0;
        }
        if (fclose(fp)) {
            set_err_msg("failed to fclose output file: %s", file);
            return 0;
        }
        if (!output_file_exists  &&  unlink(file) != 0) {
            set_err_msg("failed to remove temporary output file: %s",
                file);
            return 0;
        }
    }

    /* Check that layout and data file are readable. */
    if (params->layout_file == NULL  ||  params->data_file == NULL) {
        set_err_msg("missing command-line argument: FILE");
        return 0;
    }
    file = params->layout_file;
    if ((fp = fopen(file, "rb")) == NULL) {
        set_err_msg("failed to open file for reading: %s", file);
        return 0;
    }
    if (fclose(fp)) {
        set_err_msg("failed to close file: %s\n", file);
        return 0;
    }
    file = params->data_file;
    if ((fp = fopen(file, "rb")) == NULL) {
        set_err_msg("failed to open file for reading: %s", file);
        return 0;
    }
    if (fclose(fp)) {
        set_err_msg("failed to close file: %s\n", file);
        return 0;
    }

    return 1;
}

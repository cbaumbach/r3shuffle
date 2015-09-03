#ifndef PARSE_DATA_FILE_H
#define PARSE_DATA_FILE_H

#include "parse_layout_file.h"
#include "parse_command_line_args.h"

void offset2index(unsigned long offset, int *snp, int *trait,
    struct Layout *layout);

void index2offset(int snp, int trait, unsigned long *offset,
    struct Layout *layout);

int parse_data_file(struct Params *params, struct Layout *layout);

#endif  /* PARSE_DATA_FILE_H */

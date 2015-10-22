#include "IO.h"
#include <stdio.h>

FILE *(*IO_OpenFile)(const char *, const char *) = fopen;
int (*IO_CloseFile)(FILE *) = fclose;

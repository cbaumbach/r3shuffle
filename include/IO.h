#ifndef IO_H
#define IO_H

#include <stdio.h>

extern FILE *(*IO_OpenFile)(const char *, const char *);
extern int (*IO_CloseFile)(FILE *);

#endif

#ifndef IOSPY_H
#define IOSPY_H

#include "IO.h"

FILE *IOSpy_OpenFile(const char *, const char *);
int IOSpy_CloseFile(FILE *);

void IOSpy_OpenFileReturnNULL(void);
void IOSpy_OpenFileReturnNonNULL(void);

#endif

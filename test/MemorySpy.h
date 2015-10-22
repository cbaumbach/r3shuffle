#ifndef MEMORYSPY_H
#define MEMORYSPY_H

#include "Memory.h"
#include <stddef.h>

void *MemorySpy_Malloc(size_t);
void MemorySpy_MallocReset(void);
void MemorySpy_MallocReturnNULL(void);

#endif

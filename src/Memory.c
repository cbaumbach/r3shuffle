#include "Memory.h"
#include <stdlib.h>

void *(*Memory_Malloc)(size_t) = malloc;

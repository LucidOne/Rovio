#ifndef __CMP_COMMON_H__
#define __CMP_COMMON_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define PRINT_MEM_OUT do {fprintf (stderr, "No memory: %s %d\n", __FILE__, __LINE__);} while (0)
#define PTE do {fprintf (stderr, "Error in %s %d\n", __FILE__, __LINE__);} while (0)

#endif


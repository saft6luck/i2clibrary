/*
   i2c.library dummy function and version string file.
*/

#include <exec/types.h>
#include "lib_version.h"

LONG dummy_function(void)
{
	return -1;
}

__attribute__ ((section(".text"))) const char VTag[] = VERSTAG;

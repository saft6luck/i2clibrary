
/****i* i2c.library/FreeI2C *************************************************
*
* NAME
*   FreeI2C -- backward compatibility function (V37)
*
* SYNOPSIS
*   FreeI2C()
*
*   void FreeI2C(void);
*
* FUNCTION
*
* INPUTS
*   None.
*
* RESULT
*   None.
*
*****************************************************************************
*
*/

#include "library.h"

#include <libraries/i2c.h>


__saveds void LibFreeI2C(struct MyLibBase *base __asm("a6"))
{
	return;
}

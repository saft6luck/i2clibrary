
/****** i2c.library/SetI2CDelay *********************************************
*
* NAME
*   SetI2CDelay -- controls I2C data transmission speed (V37)
*
* SYNOPSIS
*   olddelay = SetI2CDelay(ticks)
*                          D0
*
*   ULONG SetI2CDelay(ULONG ticks);
*
* FUNCTION
*
* INPUTS
*
* RESULT
*
* SEE ALSO
*
*****************************************************************************
*
*/

#include "library.h"

#include <libraries/i2c.h>


__saveds ULONG LibSetI2CDelay(struct MyLibBase *base __asm("a6"), ULONG ticks __asm("d0"))
{
	if(FALSE && (base == NULL)) {};
	return ticks;
}


/****i* i2c.library/AllocI2C ************************************************
*
* NAME
*   AllocI2C -- {short} (V40)
*
* SYNOPSIS
*   BYTE AllocI2C(UBYTE delaytype, STRPTR name)
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


__saveds BYTE LibAllocI2C(struct MyLibBase *base __asm("a6"), UBYTE delaytype __asm("d0"), STRPTR name __asm("a0"))
{
	UBYTE dummy;
	if(FALSE && (base == NULL)) {};
	if(FALSE && (name == NULL)) {};
	dummy = base->LibGlobal.I2C_CurrentOperationMode;
	dummy = delaytype;
	return dummy;
}

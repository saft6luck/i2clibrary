
/****** i2c.library/I2CErrText **********************************************
*
* NAME
*   I2CErrText -- returns text description of an error code (V39)
*
* SYNOPSIS
*   text = I2CErrText(errnum)
*                     D0
*
*   STRPTR I2CErrText(ULONG errnum);
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


__saveds STRPTR LibI2CErrText(struct MyLibBase *base __asm("a6"),
	ULONG errnum __asm("d0"))
{
}

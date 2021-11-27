
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

#include <stdio.h>
#include <stdlib.h>

#include <proto/exec.h>
#include <proto/utility.h>
//#include <proto/dos.h>

__saveds STRPTR LibI2CErrText(struct MyLibBase *base __asm("a6"),
	ULONG errnum __asm("d0"))
{
	STRPTR stringpointer;

	//base->errortext_magic = 2342134UL;
//	stringpointer = NULL;
	stringpointer = (STRPTR)AllocVec(256, 0);

//	strcpy(stringpointer, "Error code 0x");

	sprintf((char *)stringpointer, "ErrorCode 0x%08lu", errnum);

	return stringpointer;
}

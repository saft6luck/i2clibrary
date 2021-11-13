
/****** i2c.library/SendI2C *************************************************
*
* NAME
*   SendI2C -- sends data packet on I2C bus (V37)
*
* SYNOPSIS
*   error = SendI2C(addr, number, data)
*                   D0    D1      A1
*
*   ULONG SendI2C(UBYTE addr, UWORD number, UBYTE* data)
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


__saveds ULONG LibSendI2C(struct MyLibBase *base __asm("a6"),
	UBYTE addr __asm("d0"), UWORD number __asm("d1"), UBYTE* data __asm("a1"))
{
	base->send_magic = addr;
	base->send_magic <<= 8;
	base->send_magic |= number;
	base->send_magic <<= 16;
	base->send_magic |= data[0];
	base->send_magic <<= 8;
	base->send_magic |= data[1];

	return base->send_magic;
}

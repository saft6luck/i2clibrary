
/****** i2c.library/ReceiveI2C **********************************************
*
* NAME
*   ReceiveI2C -- receives packet of data from I2C bus (V37)
*
* SYNOPSIS
*   error = ReceiveI2C(addr, number, data)
*                      D0    D1      A1
*
*   ULONG ReceiveI2C(UBYTE addr, UWORD number, UBYTE* data);
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


__saveds ULONG LibReceiveI2C(struct MyLibBase *base __asm("a6"),
	UBYTE addr __asm("d0"), UWORD number __asm("d1"), UBYTE* data __asm("a1"))
{
	base->receive_magic = addr;
	base->receive_magic <<= 16;
	base->receive_magic |= number;
	base->receive_magic <<= 8;
	base->receive_magic |= data[0];
	base->receive_magic <<= 8;
	base->receive_magic |= data[1];

	return base->receive_magic;
}

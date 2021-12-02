
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
	//LibInitI2C();
	//ctrl = I2CCON_CR_330KHZ | I2CCON_ENSIO;
	clockport_write(&base->sc, I2CCON, I2CCON_CR_330KHZ | I2CCON_ENSIO);
	//Delay(5);
	pca9564_write(&(base->sc), addr, number, &data);

	if (base->sc.cur_result == RESULT_OK) {
		return addr; //I2C_OK;
	} else {
		//		error - may be considered as three UBYTE's: 0x00AABBCC, with
		//						CC: Zero, if an error occurred.
		//						BB: I/O error number (see i2c_library.h)
		//						AA: Allocation error number (see i2c_library.h)
    //I2C_REJECT=1,        Data not acknowledged (i.e. unwanted)
    //I2C_NO_REPLY,        Chip address apparently invalid
    //SDA_TRASHED,         SDA line randomly trashed. Timing problem?
    //SDA_LO,              SDA always LO \_wrong interface attached,
    //SDA_HI,              SDA always HI / or none at all?
    //SCL_TIMEOUT,         \_Might make sense for interfaces that can
    //SCL_HI,              / read the clock line, but currently none can.
    //I2C_HARDW_BUSY       Hardware allocation failed
		return (1 << 8); //(I2C_REJECT << 8);
	}
}

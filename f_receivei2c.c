
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
	//LibInitI2C();
	//ctrl = I2CCON_CR_330KHZ | I2CCON_ENSIO;
	clockport_write(&base->sc, I2CCON, I2CCON_CR_330KHZ | I2CCON_ENSIO);
	//Delay(5);
	pca9564_read(&base->sc, addr, number, &data);

	if (base->sc.cur_result == RESULT_OK) {
		return number; //I2C_OK;
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
		switch(base->sc.cur_result) {
				case 2: //RESULT_NACK:
					return (2 << 8);
					break;
			  case 3: //RESULT_ARBLOST:
					return (3 << 8);
					break;
				default:
					return (4 << 8);
				break;
		}
	}
}

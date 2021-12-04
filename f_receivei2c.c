
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
	// Note that a master should not transmit its own slave address
	if((addr & 0x80) || !addr)
		return 2 << 8;
	//LibInitI2C();
	//ctrl = I2CCON_CR_330KHZ | I2CCON_ENSIO;
	clockport_write(&base->sc, I2CCON, I2CCON_CR_330KHZ | I2CCON_ENSIO);

	pca9564_read(&base->sc, addr, number, &data);

	if (base->sc.cur_result == RESULT_OK) {
		return number; //I2C_OK;
	} else {
		//		error - may be considered as three UBYTE's: 0x00AABBCC, with
		//						CC: Zero, if an error occurred.
		//						BB: I/O error number (see i2c_library.h)
		//						AA: Allocation error number (see i2c_library.h)
		//RESULT_OK=0,        /* Last send/receive was OK */
		//RESULT_REJECT=1,       /* Data not acknowledged (i.e. unwanted) */
		//RESULT_NO_REPLY=2,      /* Chip address apparently invalid */
		//RESULT_SDA_TRASHED=3,
		//RESULT_SDA_LO=4,   /* SDA always LO \_wrong interface attached, */
		//RESULT_SDA_HI=5,
		//RESULT_SCL_TIMEOUT=6,
		//RESULT_SCL_HI=7,
		//RESULT_HARDW_BUSY=8
    //I2C_REJECT=1,        Data not acknowledged (i.e. unwanted)
    //I2C_NO_REPLY,        Chip address apparently invalid
    //SDA_TRASHED,         SDA line randomly trashed. Timing problem?
    //SDA_LO,              SDA always LO \_wrong interface attached,
    //SDA_HI,              SDA always HI / or none at all?
    //SCL_TIMEOUT,         \_Might make sense for interfaces that can
    //SCL_HI,              / read the clock line, but currently none can.
    //I2C_HARDW_BUSY       Hardware allocation failed

		return ((base->sc.cur_result & 0xff) << 8);

		switch(base->sc.cur_result) {
				case 2: //RESULT_NACK:
					return (2 << 8);
					break;
			  case 3: //RESULT_ARBLOST:
					return (3 << 8);
					break;
				default:
					return ((base->sc.cur_result & 0xff) << 8);
				break;
		}
	}
}

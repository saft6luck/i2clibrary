
/****i* i2c.library/InitI2C *************************************************
*
* NAME
*   InitI2C -- Initializes PCA9665 chip (V37)
*
* SYNOPSIS
*   InitI2C()
*
*   void InitI2C(void);
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


__saveds void LibInitI2C(struct MyLibBase *base __asm("a6"))
{
	//ctrl = I2CCON_CR_330KHZ | I2CCON_ENSIO;
	clockport_write(&base->sc, I2CCON, base->sc.cr | I2CCON_ENSIO);
}


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
	// set I2C to requested clock rate and activate internal oscilator
	if (IS_PCA9665(base->sc.pca_type)) { 
		clockport_write(&base->sc, PCA9665_SCLL, base->sc.PCA_ClockRate_low);
		clockport_write(&base->sc, PCA9665_SCLH, base->sc.PCA_ClockRate_high);
		clockport_write(&base->sc, PCA9665_MODE, base->sc.PCA_Mode);
		clockport_write(&base->sc, PCA9665_CON, PCA9665_CON_ENSIO);
	} else  {
		clockport_write(&base->sc, PCA9564_I2C_PCA_CON, base->sc.PCA_ClockRate_low | PCA9564_I2C_PCA_CON_ENSIO);
	}

}

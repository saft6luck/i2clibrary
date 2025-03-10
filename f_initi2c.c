
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


__saveds void LibInitI2C(struct MyLibBase *LibBase __asm("a6"))
{
	/* init HW */
	HW_init(&LibBase->LibGlobal);
	/* TODO: set I2C status */

}


/****** i2c.library/ShutDownI2C *********************************************
*
* NAME
*   ShutDownI2C -- disables I2C operations (V39)
*
* SYNOPSIS
*   ShutDownI2C()
*
*   void ShutDownI2C(void);
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


__saveds void LibShutDownI2C(struct MyLibBase *base __asm("a6"))
{
	base->shutdown_magic = 234234UL;
}

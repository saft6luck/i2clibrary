
/****** i2c.library/BringBackI2C ********************************************
*
* NAME
*   BringBackI2C -- reallocates and reenables I2C hardware (V39)
*
* SYNOPSIS
*   error = BringBackI2C()
*
*   BYTE BringBackI2C(void);
*
* FUNCTION
*
* INPUTS
*   None.
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


__saveds BYTE LibBringBackI2C(struct MyLibBase *base __asm("a6"))
{
	if(FALSE && (base == NULL)) {};
	return 0;
}

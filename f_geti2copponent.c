
/****** i2c.library/GetI2COpponent ******************************************
*
* NAME
*   GetI2COpponent -- get name of task using our hardware (V39)
*
* SYNOPSIS
*   name = GetI2COpponent()
*
*   STRPTR GetI2COpponent(void);
*
* FUNCTION
*   Returns name of task currently blocking resources needed for I2C access.
*   If these resources are not blocked, NULL is returned. Akuhei I2C master
*   is connected to Amiga clock port, which does not support concept of
*   locking. As no task may "own" the clock port, this function always
*   returns NULL.
*
* INPUTS
*   None.
*
* RESULT
*   Always NULL.
*
* SEE ALSO
*   BringBackI2C(), ShutDownI2C()
*
*****************************************************************************
*
*/

#include "library.h"

#include <libraries/i2c.h>


__saveds STRPTR LibGetI2COpponent(struct MyLibBase *base __asm("a6"))
{
	return NULL;
}

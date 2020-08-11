/*
   i2c.library internal library definitions.
   Generated with LibMaker 0.12.
*/

#ifndef I2C_LIBRARY_H
#define I2C_LIBRARY_H

#include <exec/libraries.h>
#include <exec/semaphores.h>


struct MyLibBase
{
	struct Library          LibNode;
	APTR                    Seglist;
	struct SignalSemaphore  BaseLock;
	BOOL                    InitFlag;
};

#endif      /* I2C_LIBRARY_H */

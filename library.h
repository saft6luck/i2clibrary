/*
   i2c.library internal library definitions.
   Generated with LibMaker 0.12.
*/

#ifndef I2C_LIBRARY_H
#define I2C_LIBRARY_H

#include <exec/libraries.h>
#include <exec/semaphores.h>
#include "akuhei2c.h"

struct MyLibBase
{
	struct Library          LibNode;
	APTR                    Seglist;
	struct SignalSemaphore  BaseLock;
	BOOL                    InitFlag;

	pca9564_state_t         sc;
  struct Interrupt        *int6;
};

#endif      /* I2C_LIBRARY_H */

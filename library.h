/*
   i2c.library internal library definitions.
   Generated with LibMaker 0.12.
*/

#ifndef I2C_LIBRARY_H
#define I2C_LIBRARY_H

#include <exec/libraries.h>
#include <exec/semaphores.h>
#include "library_common.h"
#include "PCA9665.h"
#include "PCA9564.h"

struct MyLibBase
{
	struct Library          LibNode;
	APTR                    Seglist;
	struct SignalSemaphore  BaseLock;
	BOOL                    InitFlag;

	I2C_state_t             sc;
  struct Interrupt         *int6;
};


#define CLOCKPORT_BASE		0xD9C001  /* A4000 I2C address */
#define CLOCKPORT_STEPSIZE         2  /* A2-A3 */


#endif      /* I2C_LIBRARY_H */

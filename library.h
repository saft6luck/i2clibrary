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

	ULONG                   first_added_field;
	ULONG                   initialized_magic;
	ULONG                   open_magic;

	ULONG                   bringback_magic;
	ULONG                   errortext_magic;
	ULONG                   shutdown_magic;
	ULONG                   getopponent_magic;
	ULONG                   receive_magic;
	ULONG                   send_magic;
	ULONG                   init_magic;
	ULONG                   setdelay_magic;
	ULONG                   free_magic;
	ULONG                   freeresources_magic;
	ULONG                   alloc_magic;

	pca9564_state_t         sc;
  struct Interrupt        *int6;
};

#endif      /* I2C_LIBRARY_H */

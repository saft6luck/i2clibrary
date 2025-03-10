/****** i2c.library/background **********************************************
*
* DESCRIPTION
*
* HISTORY
*
*****************************************************************************
*
*/

#include <dos/dos.h>
#include <dos/var.h>

#define __NOLIBBASE__

#include <proto/exec.h>

#define STRPTR_TYPEDEF

#include <exec/resident.h>
#include <exec/libraries.h>
#include <clib/alib_protos.h>

#define UNUSED __attribute__((unused))

#include "lib_version.h"
#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <exec/memory.h>

#include "PCA9665.h"

#include <clib/exec_protos.h>
#include <clib/expansion_protos.h>
#include <libraries/configvars.h>
#include <dos/rdargs.h>

const char LibName[] = LIBNAME;
extern const char VTag[];

struct Library    *SysBase;
struct DosLibrary *DOSBase = NULL;
struct Library    *ExpansionBase = NULL;

__saveds struct Library *LibInit(APTR seglist __asm("a0"), struct Library *sysbase __asm("a6"));
__saveds struct Library *LibOpen(struct MyLibBase *LibBase __asm("a6"));
__saveds ULONG LibClose(struct MyLibBase *LibBase __asm("a6"));
__saveds APTR LibExpunge(struct MyLibBase *LibBase __asm("a6"));
__saveds ULONG LibReserved(void);


BYTE LibAllocI2C(struct MyLibBase *LibBase, UBYTE delaytype, STRPTR name);
void LibFreeI2C(struct MyLibBase *LibBase);
ULONG LibSetI2CDelay(struct MyLibBase *LibBase, ULONG ticks);
void LibInitI2C(struct MyLibBase *LibBase);
ULONG LibSendI2C(struct MyLibBase *LibBase, UBYTE addr, UWORD number, UBYTE* data);
ULONG LibReceiveI2C(struct MyLibBase *LibBase, UBYTE addr, UWORD number, UBYTE* data);
STRPTR LibGetI2COpponent(struct MyLibBase *LibBase);
STRPTR LibI2CErrText(struct MyLibBase *LibBase, ULONG errnum);
void LibShutDownI2C(struct MyLibBase *LibBase);
BYTE LibBringBackI2C(struct MyLibBase *LibBase);

/* Start location for I2C address in A4000D+ */
UBYTE *cps[] = { (UBYTE *)CLOCKPORT_BASE}; 

/* This function can detect and distinguish PCA9564 and PCA9664. */
/* result: TRUE when a PCA was found, else FALSE                 */
/* global: LibGlobal->pca_type = PCA_9665 or PCA_9564 */
BOOL detect_pca(I2C_state_t *LibGlobal)
{
	BOOL result = FALSE;
	
	/* A PCA9564 or PCA9665 is in usable state on startup only if it is IDLE 0xF8 */
	if(clockport_read(LibGlobal, PCA9665_STA) == PCA9665_STA_IDLE)
    {
		/* For PCA 9564 and PCA 9665 DATA register should be 0x00 */
		if(clockport_read(LibGlobal, PCA9665_DAT) == 0x00)
		{
			/* write to DATA register to see if writing is possible */
			clockport_write(LibGlobal, PCA9665_DAT, 0xCC);
			
			if(clockport_read(LibGlobal, PCA9665_DAT) == 0xCC)
			{
				/* DATA register is fine */
				/* try ADR now */
				
				/* Init PCA9665_INDPTR (TO for PCA9564) to ensure a read or write to PCA9665_INDIRECT will go to PCA9665_ADR */
				clockport_write(LibGlobal, PCA9665_INDPTR, PCA9665_ADR);
				
				/* First check for PCA9665 */
				if(clockport_read(LibGlobal, PCA9665_INDIRECT) == PCA9665_ADR_DEFAULT)
				{
					/* assume we found a PCA9665 */
					LibGlobal->pca_type = PCA_9665;
					result = TRUE;

					/* restore PCA9665_INDPTR  register */
					clockport_write(LibGlobal, PCA9665_INDPTR, 0x00);

				} else {

					/* restore PCA9564 TO register */
					clockport_write(LibGlobal, PCA9665_INDPTR, 0xFF);

					/* continue check for PCA9564 */
					/* ADR MUST be even */
					clockport_write(LibGlobal, PCA9665_INDIRECT, 0x44);
				
					if(clockport_read(LibGlobal, PCA9665_INDIRECT) == 0x44)
					{
				        /* assume we found a PCA9564 */
						LibGlobal->pca_type = PCA_9564;
						result = TRUE;
					} else {
						/* no PCA found */
						LibGlobal->pca_type = PCA_UNKNOWN;
					}

					/* restore ADR */
					clockport_write(LibGlobal, PCA9665_INDIRECT, 0x00);
				}
			}
			/* restore DATA register */
			clockport_write(LibGlobal, PCA9665_DAT, 0x00);
		} 
	}
	
	return result;

}

UBYTE atoh(char c) {
	UBYTE r;
	if ((c <='9') && (c >= '0')) {
		r = c - '0';
	} else {
		r = 10 + c;
		if ((c <= 'F') && (c >= 'A'))
			r -= 'A';
		else
			r -= 'a';
	}
	return r;
}

BOOL InitResources(struct MyLibBase *LibBase)
{
	BOOL detected = FALSE;
	UBYTE k = 0;

	/* default values for the I2C controller */
	/* depending on the selected (default) HW variant */
	if (IS_PCA9665(LibBase->LibGlobal.pca_type)) {
		/* PCA9665 */
		LibBase->LibGlobal.PCA_ClockRate_low  = PCA9665_SCLL_CR_100KHZ_LOW;
		LibBase->LibGlobal.PCA_ClockRate_high = PCA9665_SCLH_CR_100KHZ_HIGH;
		LibBase->LibGlobal.PCA_Mode           = PCA9665_MODE_CR_100KHZ_MODE;
	} else {
		/* all other cases: PCA9564 */
		LibBase->LibGlobal.PCA_ClockRate_low = PCA9564_I2CCON_CR_330KHZ;
		LibBase->LibGlobal.PCA_ClockRate_high = 0x00;
		LibBase->LibGlobal.PCA_Mode = 0x00;
	}

	LibBase->LibGlobal.CP_Address =   (UBYTE*)CLOCKPORT_BASE;
	LibBase->LibGlobal.CP_StepSize = CLOCKPORT_STEPSIZE;
	LibBase->LibGlobal.I2C_CurrentOperationMode = OP_NOP;

	// autodetect at the clockport
	for(k = 0; k < sizeof(cps)/sizeof(UBYTE*); ++k)
	{
		LibBase->LibGlobal.CP_Address = cps[k];
		if(detect_pca(&LibBase->LibGlobal))
		{ detected = TRUE; break; }
	}
	
	if((detected != TRUE) && (k == sizeof(cps)/sizeof(UBYTE*)))
	{
		// detect clock port on GARY PLCC socket
		// A0->A12, A1->A13, data lines D24...D31
		LibBase->LibGlobal.CP_StepSize = 12;
		LibBase->LibGlobal.CP_Address = (UBYTE*)0xD80000;
		if(detect_pca(&LibBase->LibGlobal)) { detected = TRUE; }
	}

	if(detected == TRUE)
	{

		LibBase->LibGlobal.sig_intr = -1;
		if ((LibBase->LibGlobal.sig_intr = AllocSignal(-1)) == -1) {
			return FALSE;
		}

		LibBase->LibGlobal.sigmask_intr = 1L << LibBase->LibGlobal.sig_intr;

		LibBase->LibGlobal.MainTask = FindTask(NULL);

		LibBase->int6 = AllocMem(sizeof(struct Interrupt), MEMF_PUBLIC|MEMF_CLEAR);

		if(LibBase->int6) {

			if (IS_PCA9665(LibBase->LibGlobal.pca_type)) {
				/* PCA9665 */
				LibBase->int6->is_Node.ln_Type = NT_INTERRUPT;
				LibBase->int6->is_Node.ln_Pri = -60;
				LibBase->int6->is_Node.ln_Name = "PCA9665";
				LibBase->int6->is_Data = (APTR)&(LibBase->LibGlobal);
				LibBase->int6->is_Code = (void*)pca9665_isr;
			} else {
				/* all other cases: PCA9564 */
				LibBase->int6->is_Node.ln_Type = NT_INTERRUPT;
				LibBase->int6->is_Node.ln_Pri = -60;
				LibBase->int6->is_Node.ln_Name = "PCA9564";
				LibBase->int6->is_Data = (APTR)&(LibBase->LibGlobal);
				LibBase->int6->is_Code = (void*)pca9564_isr;
			}

			AddIntServer(INTB_EXTER, LibBase->int6);

		} else {

			FreeSignal(LibBase->LibGlobal.sig_intr);
			detected = FALSE; // I2C_NO_MISC_RESOURCE;

		}
	}

	return detected;
}


VOID FreeResources(struct MyLibBase *LibBase)
{
	RemIntServer(INTB_EXTER, LibBase->int6);
	FreeMem(LibBase->int6, sizeof(struct Interrupt));
	FreeSignal(LibBase->LibGlobal.sig_intr);
}


struct Resident ROMTag =
{
	RTC_MATCHWORD,
	&ROMTag,
	&ROMTag + 1,
	0,
	VERSION,
	NT_LIBRARY,
	0,
	(char*)LibName,
	VSTRING,
	(APTR)LibInit
};


APTR JumpTable[] =
{
	(APTR)LibOpen,
	(APTR)LibClose,
	(APTR)LibExpunge,
	(APTR)LibReserved,
	(APTR)LibAllocI2C,
	(APTR)LibFreeI2C,
	(APTR)LibSetI2CDelay,
	(APTR)LibInitI2C,
	(APTR)LibSendI2C,
	(APTR)LibReceiveI2C,
	(APTR)LibGetI2COpponent,
	(APTR)LibI2CErrText,
	(APTR)LibShutDownI2C,
	(APTR)LibBringBackI2C,
	(APTR)0xFFFFFFFF,
};


__saveds struct Library* LibInit(APTR seglist __asm("a0"), struct Library *sysbase __asm("a6"))
{
	struct MyLibBase *LibBase = NULL;

	SysBase = sysbase;

	if ((LibBase = (struct MyLibBase*)MakeLibrary(JumpTable, NULL, NULL, sizeof(struct MyLibBase), 0)))
	{
		LibBase->LibNode.lib_Node.ln_Type = NT_LIBRARY;
		LibBase->LibNode.lib_Node.ln_Name = ROMTag.rt_Name;
		LibBase->LibNode.lib_Flags = LIBF_CHANGED | LIBF_SUMUSED;
		LibBase->LibNode.lib_Version = VERSION;
		LibBase->LibNode.lib_Revision = REVISION;
		LibBase->LibNode.lib_IdString = ROMTag.rt_IdString;
		LibBase->LibNode.lib_OpenCnt = 0;
		LibBase->Seglist = seglist;
		InitSemaphore(&LibBase->BaseLock);
		AddLibrary((struct Library*)LibBase);
	}

	return (struct Library*)LibBase;
}


__saveds struct Library* LibOpen(struct MyLibBase* LibBase __asm("a6"))
{
	struct Library *lib = (struct Library*)LibBase;

	ObtainSemaphore(&LibBase->BaseLock);

	if (!LibBase->InitFlag)
	{
		if (InitResources(LibBase))
			LibBase->InitFlag = TRUE;
		else
		{
			FreeResources(LibBase);
			lib = NULL;
		}
	}

	if (lib)
	{
		LibBase->LibNode.lib_Flags &= ~LIBF_DELEXP;
		LibBase->LibNode.lib_OpenCnt++;
	}

	ReleaseSemaphore(&LibBase->BaseLock);
	if (!lib) LibExpunge(LibBase);
	return lib;
}


__saveds ULONG LibClose(struct MyLibBase *LibBase __asm("a6"))
{
	ULONG ret = 0;

	ObtainSemaphore(&LibBase->BaseLock);

	if (--LibBase->LibNode.lib_OpenCnt == 0)
	{
		if (LibBase->LibNode.lib_Flags & LIBF_DELEXP)
			ret = (ULONG)LibExpunge(LibBase);
	}

	if (ret == 0) ReleaseSemaphore(&LibBase->BaseLock);
	return ret;
}


__saveds APTR LibExpunge(struct MyLibBase *LibBase __asm("a6"))
{
	APTR seglist = NULL;

	ObtainSemaphore(&LibBase->BaseLock);

	if (LibBase->LibNode.lib_OpenCnt == 0)
	{
		FreeResources(LibBase);
		Forbid();
		Remove((struct Node*)LibBase);
		Permit();
		seglist = LibBase->Seglist;
		FreeMem((UBYTE*)LibBase - LibBase->LibNode.lib_NegSize, LibBase->LibNode.lib_NegSize + LibBase->LibNode.lib_PosSize);
		LibBase = NULL;    /* freed memory, no more valid */
	}
	else LibBase->LibNode.lib_Flags |= LIBF_DELEXP;

	if (LibBase) ReleaseSemaphore(&LibBase->BaseLock);
	return seglist;
}


__saveds ULONG LibReserved(void)
{
	return 0;
}

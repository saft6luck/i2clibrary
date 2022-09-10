/*
   i2c.library, library skeleton
*/

/****** i2c.library/background **********************************************
*
* DESCRIPTION
*
* HISTORY
*
*****************************************************************************
*
*/


#define __NOLIBBASE__

#include <proto/exec.h>

//typedef char* STRPTR;
//typedef const char* CONST_STRPTR;
#define STRPTR_TYPEDEF

#include <exec/resident.h>
#include <exec/libraries.h>
#include <clib/alib_protos.h>

#define UNUSED __attribute__((unused))

#include "lib_version.h"
#include "library.h"
#include "akuhei2c.h"

const char LibName[] = LIBNAME;
extern const char VTag[];

struct Library *SysBase;


__saveds struct Library *LibInit(APTR seglist __asm("a0"), struct Library *sysbase __asm("a6"));
__saveds struct Library *LibOpen(struct MyLibBase *base __asm("a6"));
__saveds ULONG LibClose(struct MyLibBase *base __asm("a6"));
__saveds APTR LibExpunge(struct MyLibBase *base __asm("a6"));
__saveds ULONG LibReserved(void);


BYTE LibAllocI2C(struct MyLibBase *base, UBYTE delaytype, STRPTR name);
void LibFreeI2C(struct MyLibBase *base);
ULONG LibSetI2CDelay(struct MyLibBase *base, ULONG ticks);
void LibInitI2C(struct MyLibBase *base);
ULONG LibSendI2C(struct MyLibBase *base, UBYTE addr, UWORD number, UBYTE* data);
ULONG LibReceiveI2C(struct MyLibBase *base, UBYTE addr, UWORD number, UBYTE* data);
STRPTR LibGetI2COpponent(struct MyLibBase *base);
STRPTR LibI2CErrText(struct MyLibBase *base, ULONG errnum);
void LibShutDownI2C(struct MyLibBase *base);
BYTE LibBringBackI2C(struct MyLibBase *base);

UBYTE *cps[] = { (UBYTE *)0xD80001, (UBYTE *)0xD84001, (UBYTE *)0xD88001, (UBYTE *)0xD8C001, (UBYTE *)0xD90001 };
/* Prisma Megamix Zorro card with clockport */
/* UBYTE *cps[] = { (UBYTE *)0x00EA4000 }; */

BOOL detect_pca(pca9564_state_t *sc)
{
	/* PCA9564 is in usable state on startup only if it is IDLE 0xF8 */
	if(clockport_read(sc, I2CSTA) != I2CSTA_IDLE)
	 	return FALSE;

	/* upon startup ADDR and DATA regs should be NULL */
	if(clockport_read(sc, I2CADR) != 0)
		return FALSE;
	if(clockport_read(sc, I2CDAT) != 0)
		return FALSE;

	clockport_write(sc, I2CDAT, 0xCC);
	/* ADDR MUST be even */
	clockport_write(sc, I2CADR, 0x44);
	if((clockport_read(sc, I2CDAT) == 0xCC) && (clockport_read(sc, I2CADR) == 0x44))
	{
		/* restore */
		clockport_write(sc, I2CADR, 0x00);
		clockport_write(sc, I2CDAT, 0x00);
		return TRUE;
	}
	return FALSE;
}

BOOL InitResources(struct MyLibBase *base)
{
	UBYTE k, s;
	base->sc.cur_op = OP_NOP;

	/*base->sc.cp = (UBYTE *)CLOCKPORT_BASE;*/
	base->sc.stride = CLOCKPORT_STRIDE;
	for(k = 0; k < sizeof(cps)/sizeof(UBYTE*); ++k) {
		base->sc.cp = cps[k];
		if(detect_pca(&base->sc)) {
			break;
		}
	}
	if(k == sizeof(cps)/sizeof(UBYTE*)) {
		/* detect clock port on GARY PLCC socket
		   A0-A12, A1-A13, data lines D8...D15*/
		base->sc.stride = 12;
		base->sc.cp = 0xD80001;
		if(!detect_pca(&base->sc)) {
			/* Prisma Megamix Zorro card with clockport */
			base->sc.stride = 2;
			base->sc.cp = (UBYTE *)0x00EA4000;
			if(!detect_pca(&base->sc)) {
				return FALSE;
			}
		}
	}

	base->sc.sig_intr = -1;
	if ((base->sc.sig_intr = AllocSignal(-1)) == -1) {
		return FALSE;
	}
	base->sc.sigmask_intr = 1L << base->sc.sig_intr;

	base->sc.MainTask = FindTask(NULL);

	base->int6 = AllocMem(sizeof(struct Interrupt), MEMF_PUBLIC|MEMF_CLEAR);
	if(base->int6) {
					base->int6->is_Node.ln_Type = NT_INTERRUPT;
					base->int6->is_Node.ln_Pri = -60;
					base->int6->is_Node.ln_Name = "PCA9564";
					base->int6->is_Data = (APTR)&(base->sc);
					base->int6->is_Code = (void*)pca9564_isr;

					AddIntServer(INTB_EXTER, base->int6);
	} else {
					FreeSignal(base->sc.sig_intr);
					return FALSE; // I2C_NO_MISC_RESOURCE;
	}

	return TRUE;
}


VOID FreeResources(struct MyLibBase *base)
{
	RemIntServer(INTB_EXTER, base->int6);
	FreeMem(base->int6, sizeof(struct Interrupt));
	FreeSignal(base->sc.sig_intr);
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
	struct MyLibBase *base = NULL;

	SysBase = sysbase;

	if ((base = (struct MyLibBase*)MakeLibrary(JumpTable, NULL, NULL, sizeof(struct MyLibBase), 0)))
	{
		base->LibNode.lib_Node.ln_Type = NT_LIBRARY;
		base->LibNode.lib_Node.ln_Name = ROMTag.rt_Name;
		base->LibNode.lib_Flags = LIBF_CHANGED | LIBF_SUMUSED;
		base->LibNode.lib_Version = VERSION;
		base->LibNode.lib_Revision = REVISION;
		base->LibNode.lib_IdString = ROMTag.rt_IdString;
		base->LibNode.lib_OpenCnt = 0;
		base->Seglist = seglist;
		InitSemaphore(&base->BaseLock);
		AddLibrary((struct Library*)base);
	}

	return (struct Library*)base;
}


__saveds struct Library* LibOpen(struct MyLibBase* base __asm("a6"))
{
	struct Library *lib = (struct Library*)base;

	ObtainSemaphore(&base->BaseLock);

	if (!base->InitFlag)
	{
		if (InitResources(base))
			base->InitFlag = TRUE;
		else
		{
			FreeResources(base);
			lib = NULL;
		}
	}

	if (lib)
	{
		base->LibNode.lib_Flags &= ~LIBF_DELEXP;
		base->LibNode.lib_OpenCnt++;
	}

	ReleaseSemaphore(&base->BaseLock);
	if (!lib) LibExpunge(base);
	return lib;
}


__saveds ULONG LibClose(struct MyLibBase *base __asm("a6"))
{
	ULONG ret = 0;

	ObtainSemaphore(&base->BaseLock);

	if (--base->LibNode.lib_OpenCnt == 0)
	{
		if (base->LibNode.lib_Flags & LIBF_DELEXP)
			ret = (ULONG)LibExpunge(base);
	}

	if (ret == 0) ReleaseSemaphore(&base->BaseLock);
	return ret;
}


__saveds APTR LibExpunge(struct MyLibBase *base __asm("a6"))
{
	APTR seglist = NULL;

	ObtainSemaphore(&base->BaseLock);

	if (base->LibNode.lib_OpenCnt == 0)
	{
		FreeResources(base);
		Forbid();
		Remove((struct Node*)base);
		Permit();
		seglist = base->Seglist;
		FreeMem((UBYTE*)base - base->LibNode.lib_NegSize, base->LibNode.lib_NegSize + base->LibNode.lib_PosSize);
		base = NULL;    /* freed memory, no more valid */
	}
	else base->LibNode.lib_Flags |= LIBF_DELEXP;

	if (base) ReleaseSemaphore(&base->BaseLock);
	return seglist;
}


__saveds ULONG LibReserved(void)
{
	return 0;
}

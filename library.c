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

UBYTE *cps[] = { (UBYTE *)0xD9C001};

BOOL detect_pca(I2C_state_t *sc)
{
	BOOL result = FALSE;
	
	/* A PCA9564 or PCA9665 is in usable state on startup only if it is IDLE 0xF8 */
	if(clockport_read(sc, PCA9665_STA) == PCA9665_STA_IDLE)
    {
		/* For PCA 9564 and PCA 9665 DATA register should be 0x00 */
		if(clockport_read(sc, PCA9665_DAT) == 0x00)
		{
			/* write to DATA register to see if writing is possible */
			clockport_write(sc, PCA9665_DAT, 0xCC);
			
			if(clockport_read(sc, PCA9665_DAT) == 0xCC)
			{
				/* DATA register is fine */
				/* try ADR now */
				
				/* Init PCA9665_INDPTR (TO for PCA9564) to ensure a read or write to PCA9665_INDIRECT will go to PCA9665_ADR */
				clockport_write(sc, PCA9665_INDPTR, PCA9665_ADR);
				
				/* First check for PCA9665 */
				if(clockport_read(sc, PCA9665_INDIRECT) == PCA9665_ADR_DEFAULT)
				{
					/* assume we found a PCA9665 */
					sc->pca_type = PCA_9665;
					result = TRUE;

					/* restore PCA9665_INDPTR  register */
					clockport_write(sc, PCA9665_INDPTR, 0x00);

				} else {

					/* restore PCA9564 TO register */
					clockport_write(sc, PCA9665_INDPTR, 0xFF);

					/* continue check for PCA9564 */
					/* ADR MUST be even */
					clockport_write(sc, PCA9665_INDIRECT, 0x44);
				
					if(clockport_read(sc, PCA9665_INDIRECT) == 0x44)
					{
				        /* assume we found a PCA9564 */
						sc->pca_type = PCA_9564;
						result = TRUE;
					} else {
						/* no PCA found */
						sc->pca_type = PCA_UNKNOWN;
					}

					/* restore ADR */
					clockport_write(sc, PCA9665_INDIRECT, 0x00);
				}
			}
			/* restore DATA register */
			clockport_write(sc, PCA9665_DAT, 0x00);
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

BOOL InitResources(struct MyLibBase *base)
{
	UBYTE k = 0, s, detected = 0;
	ULONG var_input;
	UBYTE var_cp_name[] = "i2c/cpaddr";
	UBYTE var_cr_name[] = "i2c/cr";
	UBYTE var_value[] = "                ";
	UBYTE ENV_name[] = "ENV:";
	struct ConfigDev *myCD;
	UBYTE *buf;

	/* default values for the I2C controller */
	/* depending on the selected (default) HW variant */
	if (IS_PCA9665(base->sc.pca_type)) {
		/* PCA9665 */
		base->sc.PCA_ClockRate_low  = PCA9665_SCLL_CR_100KHZ_LOW;
		base->sc.PCA_ClockRate_high = PCA9665_SCLH_CR_100KHZ_HIGH;
		base->sc.PCA_Mode           = PCA9665_MODE_CR_100KHZ_MODE;
	} else {
		/* all other cases: PCA9564 */
		base->sc.PCA_ClockRate_low = PCA9564_I2CCON_CR_330KHZ;
		base->sc.PCA_ClockRate_high = 0x00;
		base->sc.PCA_Mode = 0x00;
	}

	base->sc.CP_Address =   (UBYTE*)CLOCKPORT_BASE;
	base->sc.CP_StepSize = CLOCKPORT_STEPSIZE;
	base->sc.I2C_CurrentOperationMode = OP_NOP;

	struct DosLibrary *oldDOSBase = DOSBase;
	struct Library   *oldExpansionBase = ExpansionBase;
	/* DOS Library not yet open? */
	if(DOSBase == NULL)
		/* open Dos Library */
		DOSBase = (struct DosLibrary *)OpenLibrary("dos.library",0L);

	if(DOSBase != NULL) {
		
		// use env variable if available and content length is >9 & <12: format 01234567 90
		if (Lock(ENV_name, SHARED_LOCK) && ((k = GetVar(var_cp_name, var_value, 12, 0)) > 9) && (k < 12))
		{
			// address CP_StepSize can be as much as 30 (leading to 2^30 as granularity) 2-> making the A0 and A1 at address lines A30 and A31
						 
			buf = var_value;
			var_input = 0UL;
			// read ENV variable for CP address
			for(s = 0; s < 8; ++s, ++buf)
			{
				var_input <<= 4;
				var_input += atoh(*buf);
			}
			base->sc.CP_Address = (UBYTE*)var_input;
			
			// read ENV variable for CP step size
			buf++; // skip delimiter
			base->sc.CP_StepSize = atoh(*buf);
			if(k > 10)
			{
				buf++;
				base->sc.CP_StepSize <<= 4;
				base->sc.CP_StepSize += atoh(*buf);
			}
			
			if(detect_pca(&base->sc)) {
				 detected = 1;
		    }
		}
		else
		{ // autodetect at the clockports or zorro boards
			for(k = 0; k < sizeof(cps)/sizeof(UBYTE*); ++k)
			{
				base->sc.CP_Address = cps[k];
				if(detect_pca(&base->sc))
				{ detected = 1; break; }
			}
			if((!detected) && (k == sizeof(cps)/sizeof(UBYTE*)))
			{
				// detect clock port on GARY PLCC socket
				// A0-A12, A1-A13, data lines D8...D15
				base->sc.CP_StepSize = 12;
				base->sc.CP_Address = (UBYTE*)0xD80002;
				if(detect_pca(&base->sc)) { detected = 1; }
			}
			if(!detected && (ExpansionBase == NULL))
			{
				//ExpansionBase = OpenLibrary("expansion.library",0L);
				ExpansionBase = (struct Library*)OpenLibrary("expansion.library",0L);
			}
			if(!detected && (ExpansionBase != NULL))
			{
				k = 0;
				myCD = NULL;
				base->sc.CP_StepSize = 2;
				while(!detected && (myCD = FindConfigDev(myCD,-1L,-1L))) // search for all ConfigDevs
			{
					// Prisma Megamix Zorro card with clockport
					if((myCD->cd_Rom.er_Manufacturer == 0x0E3B)
					&& (myCD->cd_Rom.er_Product == 0x30))
					{
						base->sc.CP_Address = (UBYTE *)((UBYTE *)myCD->cd_BoardAddr + 0x00004000UL);
						if(detect_pca(&base->sc))
						{ detected = 1; break; }
					}
					// Icomp card with clockport
					if(!detected && (myCD->cd_Rom.er_Manufacturer == 0x1212))
					{
						if((myCD->cd_Rom.er_Product == 0x05) 	// 0x1212:0x05 ISDN Surfer
						|| (myCD->cd_Rom.er_Product == 0x07)  // 0x1212:0x07 VarIO
						|| (myCD->cd_Rom.er_Product == 0x0A)) // 0x1212:0x0A KickFlash
						{
							base->sc.CP_Address = (UBYTE *)((UBYTE *)myCD->cd_BoardAddr + 0x00008000UL);
							if(myCD->cd_Rom.er_Product == 0x0A) {
								// activate CP for KickFlash
								// http://wiki.icomp.de/wiki/Kickflash#using_the_clockport
								buf = base->sc.CP_Address + 0x007C;
								*buf = 0xFF;
							}
							if(detect_pca(&base->sc))
							{ detected = 1; break; }
						}
						// Buddha I-Comp
						if(!detected && (myCD->cd_Rom.er_Product == 0x00))
						{
							base->sc.CP_Address = (UBYTE *)((UBYTE *)myCD->cd_BoardAddr + 0x00000e00UL);
							if(detect_pca(&base->sc))
							{ detected = 1; break; }
						}
					}
					// 0x1212:0x17 X-Surfer
					if(!detected && (myCD->cd_Rom.er_Product == 0x17))
					{
						base->sc.CP_Address = (UBYTE *)((UBYTE *)myCD->cd_BoardAddr + 0x0000C000UL);
						if(detect_pca(&base->sc)) // Port 0
						{
							detected = 1; break;
						} else {
							base->sc.CP_Address = (UBYTE *)((UBYTE *)myCD->cd_BoardAddr + 0x0000A001UL);
							if(detect_pca(&base->sc)) // Port 1
							{ detected = 1; break; }
						}
					}
					// A1K.org Community
					// A LAN/IDE solluntion with Clockport for the Amiga ZorroII/III Slot
					// Matthias Heinrichs
					if(!detected && (myCD->cd_Rom.er_Manufacturer == 0x0A1C) && (myCD->cd_Rom.er_Product == 0x7C))
					{
						base->sc.CP_Address = (UBYTE *)((UBYTE *)myCD->cd_BoardAddr);
						if(detect_pca(&base->sc))
						{ detected = 1; break; }
					}
					// Prisma Megamix Zorro card with clockport
					/*base->sc.CP_Address = (UBYTE *)0x00EA4000;
					if(!detect_pca(&base->sc)) {
						return FALSE;
					}*/
				} // FindConfigDev()
			}
		}

		if((DOSBase != NULL) && Lock(ENV_name, SHARED_LOCK) && ((k = GetVar(var_cr_name, var_value, 2, 0)) > 0))
			{
				base->sc.PCA_ClockRate_low = atoh(*var_value) & PCA9564_I2CCON_CR_MASK;
			}

		/* do not close libs only they were already opened before */
		if((ExpansionBase != NULL) && (oldExpansionBase == NULL))
			CloseLibrary(ExpansionBase);
		if((DOSBase != NULL) && (oldDOSBase == NULL))
			CloseLibrary((struct Library *)DOSBase);

	} else {
		/* can't open DOS library */	
	}
	

	if(detected)
	{

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
	} else {
		return FALSE;

	}
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

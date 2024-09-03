#include "library_common.h"


void __restore_a4(void)
{
    __asm volatile("\tlea ___a4_init, a4");
}

UBYTE
clockport_read(I2C_state_t *sp, UBYTE reg)
{
	UBYTE v;
	UBYTE *ptr;

	ptr = sp->CP_Address + (reg << sp->CP_StepSize);
	v = *ptr;
#ifdef DEBUG
	if (!(sp->in_isr))
		D((STRPTR)"DEBUG: read %x from %p\n", (int) v, (void*) ptr);
#endif /* DEBUG */

	return v;
}



void
clockport_write(I2C_state_t *sp, UBYTE reg, UBYTE value)
{
	UBYTE *ptr;

	ptr = (sp->CP_Address) + (reg << sp->CP_StepSize);
#ifdef DEBUG
	if (!(sp->in_isr))
		D((STRPTR)"DEBUG: write %x to %p\n", (int) value, (void*) ptr);
#endif /* DEBUG */

	*ptr = value;
}

#include "library.h"

#include <libraries/i2c.h>


void __restore_a4(void)
{
    __asm volatile("\tlea ___a4_init, a4");
}

void
I2C_read(I2C_state_t *sp, UBYTE address, ULONG size, UBYTE **buf)
{
	sp->I2C_CurrentOperationMode = OP_BYTE_READ;
	if (IS_PCA9665(sp->pca_type)) { 
		pca9665_exec(sp, address | 0x01, size, buf);
	} else {
		pca9564_exec(sp, address | 0x01, size, buf);
	}
	sp->I2C_CurrentOperationMode = OP_NOP;
}


void
I2C_write(I2C_state_t *sp, UBYTE address, ULONG size, UBYTE **buf)
{
	sp->I2C_CurrentOperationMode = OP_BYTE_WRITE;

	if (IS_PCA9665(sp->pca_type)) { 
		pca9665_exec(sp, address & 0xFE, size, buf);
	} else {
		pca9564_exec(sp, address & 0xFE, size, buf);
	}
	sp->I2C_CurrentOperationMode = OP_NOP;
}

void
HW_init(I2C_state_t *sp)
{
	// set I2C to requested clock rate (defined in LibGlobal structure) and activate internal oscilator
	if (IS_PCA9665(sp->pca_type)) {
		pca9665_init(sp);
	} else  {
		pca9564_init(sp);
	}
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

inline UBYTE
clockport_read_indirect(I2C_state_t *sp, UBYTE reg)
{
	UBYTE v;
	UBYTE *ptr;

	/* Init PCA9665_INDPTR */
	ptr = (UBYTE *)((sp->CP_Address) + (PCA9665_INDPTR << sp->CP_StepSize));
#ifdef DEBUG
	if (!(sp->in_isr))
		D((STRPTR)"DEBUG: read %x from %p\n", (int) v, (void*) ptr);
#endif /* DEBUG */

	*ptr = reg;

	/* write indirect register */
	ptr = (UBYTE *)((sp->CP_Address[0]) + (PCA9665_INDIRECT << sp->CP_StepSize));
#ifdef DEBUG
	if (!(sp->in_isr))
		D((STRPTR)"DEBUG: read %x from %p\n", (int) v, (void*) ptr);
#endif /* DEBUG */

	/* read value */
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

inline void
clockport_write_indirect(I2C_state_t *sp, UBYTE reg, UBYTE value)
{
	UBYTE *ptr;

	/* Init PCA9665_INDPTR */
	ptr = (UBYTE *)((sp->CP_Address) + (PCA9665_INDPTR << sp->CP_StepSize));
#ifdef DEBUG
	if (!(sp->in_isr))
		D((STRPTR)"DEBUG: write %x to %p\n", (int) value, (void*) ptr);
#endif /* DEBUG */

	*ptr = reg;
	
	/* write indirect register */
	ptr = (UBYTE *)((sp->CP_Address) + (PCA9665_INDIRECT << sp->CP_StepSize));
#ifdef DEBUG
	if (!(sp->in_isr))
		D((STRPTR)"DEBUG: write %x to %p\n", (int) value, (void*) ptr);
#endif /* DEBUG */
	/* write value */
	*ptr = value;
}
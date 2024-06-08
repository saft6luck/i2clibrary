#include "PCA9665.h"

void __restore_a4(void)
{
    __asm volatile("\tlea ___a4_init, a4");
}

UBYTE
clockport_read(pca_state_t *sp, UBYTE reg)
{
	UBYTE v;
	UBYTE *ptr;

	ptr = sp->cp + (reg << sp->stride);
	v = *ptr;
#ifdef DEBUG
	if (!(sp->in_isr))
		D((STRPTR)"DEBUG: read %x from %p\n", (int) v, (void*) ptr);
#endif /* DEBUG */

	return v;
}

void
clockport_write(pca_state_t *sp, UBYTE reg, UBYTE value)
{
	UBYTE *ptr;

	ptr = (sp->cp) + (reg << sp->stride);
#ifdef DEBUG
	if (!(sp->in_isr))
		D((STRPTR)"DEBUG: write %x to %p\n", (int) value, (void*) ptr);
#endif /* DEBUG */

	*ptr = value;
}

void
pca9665_write(pca_state_t *sp, UBYTE address, ULONG size, UBYTE **buf)
{
	sp->cur_op = OP_WRITE;
	pca9665_exec(sp, address & 0xFE, size, buf);
	sp->cur_op = OP_NOP;
}

void
pca9665_read(pca_state_t *sp, UBYTE address, ULONG size, UBYTE **buf)
{
	sp->cur_op = OP_READ;
	pca9665_exec(sp, address | 0x01, size, buf);
	sp->cur_op = OP_NOP;
}

void
pca9665_exec(pca_state_t *sp, UBYTE address, ULONG size, UBYTE **buf)
{
	sp->slave_addr = address;
	sp->buf_size = size;
	sp->bytes_count = 0;
	sp->buf = *buf;

	pca9665_send_start(sp);

	Wait(sp->sigmask_intr);

	if (sp->cur_result != RESULT_OK) {
#ifdef DEBUG
		D((STRPTR)"OP: failed!\n");
		//pca9665_dump_state(sp);
#endif /* DEBUG */
	}

	sp->buf_size = 0;
	sp->slave_addr = 0;
}

void
pca9665_send_start(pca_state_t *sp)
{
	UBYTE c;

	c = clockport_read(sp, PCA9665_CON);
	c |= PCA9665_CON_STA|PCA9665_CON_AA;
	clockport_write(sp, PCA9665_CON, c);	/* send START condition */

}

/* Interrupt service routine. */
__saveds int pca9665_isr(pca_state_t *sp __asm("a1"))
{
	UBYTE v;

#ifdef DEBUG
  sp->in_isr = TRUE;
	sp->isr_called++;
#endif /* DEBUG */

	if (!(clockport_read(sp, PCA9665_CON) & PCA9665_CON_SI)) {
#ifdef DEBUG
		sp->in_isr = FALSE;
#endif /* DEBUG */
		return 1;
	}

  switch (sp->cur_op) {
	case OP_READ:
		switch (clockport_read(sp, PCA9665_STA)) {
		case PCA9665_STA_START_SENT:		/* 0x08 */
			clockport_write(sp, PCA9665_DAT, sp->slave_addr); // | 0x01);
			v = clockport_read(sp, PCA9665_CON);
			v &= ~(PCA9665_CON_SI|PCA9665_CON_STA);
			clockport_write(sp, PCA9665_CON, v);
			break;

		case PCA9665_STA_SLAR_TX_ACK_RX:	/* 0x40 */
			v = clockport_read(sp, PCA9665_CON);
			v &= ~(PCA9665_CON_SI);

			if ((sp->bytes_count) <= sp->buf_size)
				v |= (PCA9665_CON_AA);
			else
				v &= ~(PCA9665_CON_AA); /* last byte */
			clockport_write(sp, PCA9665_CON, v);
			break;

		case PCA9665_STA_SLAR_TX_NACK_RX:	/* 0x48 */
			v = clockport_read(sp, PCA9665_CON);
			v &= ~(PCA9665_CON_SI);
			v |= (PCA9665_CON_STO);	/* send stop */
			clockport_write(sp, PCA9665_CON, v);
			sp->cur_result = RESULT_NO_REPLY; /* NO ACK */
			Signal(sp->MainTask, sp->sigmask_intr);
			break;

		case PCA9665_STA_DATA_RX_ACK_TX:	/* 0x50 */
			sp->buf[sp->bytes_count] = clockport_read(sp, PCA9665_DAT);
			(sp->bytes_count)++;
			v = clockport_read(sp, PCA9665_CON);
			v &= ~(PCA9665_CON_SI);
			if ((sp->bytes_count+1) < sp->buf_size)
				v |= (PCA9665_CON_AA);
			else
				v &= ~(PCA9665_CON_AA); /* last byte */
			clockport_write(sp, PCA9665_CON, v);
			break;

		case PCA9665_STA_DATA_RX_NACK_TX:	/* 0x58 */
			sp->buf[sp->bytes_count] = clockport_read(sp, PCA9665_DAT);
			(sp->bytes_count)++;
			v = clockport_read(sp, PCA9665_CON);
			v &= ~(PCA9665_CON_SI);
			v |= (PCA9665_CON_AA|PCA9665_CON_STO);	/* send stop */
			clockport_write(sp, PCA9665_CON, v);
			sp->cur_result = RESULT_OK;
			Signal(sp->MainTask, sp->sigmask_intr);
			break;

		default: /**/
			clockport_write(sp, PCA9665_CON, 0);
			sp->cur_result = RESULT_HARDW_BUSY;
			Signal(sp->MainTask, sp->sigmask_intr);
			break;
		}
		break;

	case OP_WRITE:
		switch (clockport_read(sp, PCA9665_STA)) {
		case PCA9665_STA_START_SENT:		/* 0x08 */
			clockport_write(sp, PCA9665_DAT, sp->slave_addr); // & 0xFE);
			v = clockport_read(sp, PCA9665_CON);
			v &= ~(PCA9665_CON_SI|PCA9665_CON_STA);
			clockport_write(sp, PCA9665_CON, v);
			break;

		case PCA9665_STA_SLAW_TX_ACK_RX:	 /* 0x18 */
			v = clockport_read(sp, PCA9665_CON);
			v &= ~(PCA9665_CON_SI|PCA9665_CON_STA);
			if ((sp->bytes_count) < sp->buf_size) {
				clockport_write(sp, PCA9665_DAT, sp->buf[sp->bytes_count]);
			} else {
				v |= (PCA9665_CON_STO);
			}
			clockport_write(sp, PCA9665_CON, v);
			if (sp->bytes_count == sp->buf_size) {
				sp->cur_result = RESULT_OK;
				Signal(sp->MainTask, sp->sigmask_intr);
			}
			break;

		case PCA9665_STA_SLAW_TX_NACK_RX:	/* 0x20 */
      v = clockport_read(sp, PCA9665_CON);
      v |= (PCA9665_CON_STO);
      clockport_write(sp, PCA9665_CON, v);
      sp->cur_result = RESULT_NO_REPLY;
      Signal(sp->MainTask, sp->sigmask_intr);
      break;

		case PCA9665_STA_DATA_TX_ACK_RX:	/* 0x28 */
			v = clockport_read(sp, PCA9665_CON);
      (sp->bytes_count)++;
			if (sp->bytes_count < sp->buf_size) {
				clockport_write(sp, PCA9665_DAT, sp->buf[sp->bytes_count]);
			} else {
				v |= (PCA9665_CON_STO);
			}
			v &= ~(PCA9665_CON_SI);
			clockport_write(sp, PCA9665_CON, v);
			if (sp->bytes_count == sp->buf_size) {
				sp->cur_result = RESULT_OK;
				Signal(sp->MainTask, sp->sigmask_intr);
			}
			break;

		default:
			clockport_write(sp, PCA9665_CON, 0);
			sp->cur_result = RESULT_HARDW_BUSY;
			Signal(sp->MainTask, sp->sigmask_intr);
			break;
		}
		break;
	case OP_NOP:
		clockport_write(sp, PCA9665_CON, 0);
		sp->cur_result = RESULT_OK; //RESULT_HARDW_BUSY;
		Signal(sp->MainTask, sp->sigmask_intr);
		break;
	}

#ifdef DEBUG
  sp->in_isr = FALSE;
#endif /* DEBUG */
	return 0;
}

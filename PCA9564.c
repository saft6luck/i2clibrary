#include "library.h"
#include "PCA9564.h"



void
pca9564_init(I2C_state_t *sp)
{
	// set I2C to requested clock rate (defined in LibGlobal structure) and activate internal oscilator
	clockport_write(sp, PCA9564_I2C_PCA_CON, sp->PCA_ClockRate_low | PCA9564_I2C_PCA_CON_ENSIO);
}


void
pca9564_exec(I2C_state_t *sp, UBYTE address, ULONG size, UBYTE **buf)
{
	sp->slave_addr = address;
	sp->buf_size = size;
	sp->bytes_count = 0;
	sp->buf = *buf;

	pca9564_send_start(sp);

	Wait(sp->sigmask_intr);

	if (sp->cur_result != RESULT_OK) {
#ifdef DEBUG
		D((STRPTR)"OP: failed!\n");
		//pca9564_dump_state(sp);
#endif /* DEBUG */
	}

	sp->buf_size = 0;
	sp->slave_addr = 0;
}

void
pca9564_send_start(I2C_state_t *sp)
{
	UBYTE c;

	c = clockport_read(sp, PCA9564_I2CCON);
	c |= PCA9564_I2CCON_STA|PCA9564_I2CCON_AA;
	clockport_write(sp, PCA9564_I2CCON, c);	/* send START condition */

}

/* Interrupt service routine. */
__saveds int pca9564_isr(I2C_state_t *sp __asm("a1"))
{
	UBYTE v;

#ifdef DEBUG
  sp->in_isr = TRUE;
	sp->isr_called++;
#endif /* DEBUG */

	if (!(clockport_read(sp, PCA9564_I2CCON) & PCA9564_I2CCON_SI)) {
#ifdef DEBUG
		sp->in_isr = FALSE;
#endif /* DEBUG */
		return 1;
	}

	switch (sp->I2C_CurrentOperationMode) {
		case OP_BUFFER_READ:
		case OP_BYTE_READ:
			switch (clockport_read(sp, PCA9564_I2CSTA)) {
			case PCA9564_I2CSTA_START_SENT:		/* 0x08 */
				clockport_write(sp, PCA9564_I2CDAT, sp->slave_addr); // | 0x01);
				v = clockport_read(sp, PCA9564_I2CCON);
				v &= ~(PCA9564_I2CCON_SI|PCA9564_I2CCON_STA);
				clockport_write(sp, PCA9564_I2CCON, v);
				break;

			case PCA9564_I2CSTA_SLAR_TX_ACK_RX:	/* 0x40 */
				v = clockport_read(sp, PCA9564_I2CCON);
				v &= ~(PCA9564_I2CCON_SI);

				if ((sp->bytes_count) <= sp->buf_size)
					v |= (PCA9564_I2CCON_AA);
				else
					v &= ~(PCA9564_I2CCON_AA); /* last byte */
				clockport_write(sp, PCA9564_I2CCON, v);
				break;

			case PCA9564_I2CSTA_SLAR_TX_NACK_RX:	/* 0x48 */
				v = clockport_read(sp, PCA9564_I2CCON);
				v &= ~(PCA9564_I2CCON_SI);
				v |= (PCA9564_I2CCON_STO);	/* send stop */
				clockport_write(sp, PCA9564_I2CCON, v);
				sp->cur_result = RESULT_NO_REPLY; /* NO ACK */
				Signal(sp->MainTask, sp->sigmask_intr);
				break;

			case PCA9564_I2CSTA_DATA_RX_ACK_TX:	/* 0x50 */
				sp->buf[sp->bytes_count] = clockport_read(sp, PCA9564_I2CDAT);
				(sp->bytes_count)++;
				v = clockport_read(sp, PCA9564_I2CCON);
				v &= ~(PCA9564_I2CCON_SI);
				if ((sp->bytes_count+1) < sp->buf_size)
					v |= (PCA9564_I2CCON_AA);
				else
					v &= ~(PCA9564_I2CCON_AA); /* last byte */
				clockport_write(sp, PCA9564_I2CCON, v);
				break;

			case PCA9564_I2CSTA_DATA_RX_NACK_TX:	/* 0x58 */
				sp->buf[sp->bytes_count] = clockport_read(sp, PCA9564_I2CDAT);
				(sp->bytes_count)++;
				v = clockport_read(sp, PCA9564_I2CCON);
				v &= ~(PCA9564_I2CCON_SI);
				v |= (PCA9564_I2CCON_AA|PCA9564_I2CCON_STO);	/* send stop */
				clockport_write(sp, PCA9564_I2CCON, v);
				sp->cur_result = RESULT_OK;
				Signal(sp->MainTask, sp->sigmask_intr);
				break;

			default: /**/
				clockport_write(sp, PCA9564_I2CCON, 0);
				sp->cur_result = RESULT_HARDW_BUSY;
				Signal(sp->MainTask, sp->sigmask_intr);
				break;
			}
			break;

		case OP_BUFFER_WRITE:
		case OP_BYTE_WRITE:
			switch (clockport_read(sp, PCA9564_I2CSTA)) {
				case PCA9564_I2CSTA_START_SENT:		/* 0x08 */
					clockport_write(sp, PCA9564_I2CDAT, sp->slave_addr); // & 0xFE);
					v = clockport_read(sp, PCA9564_I2CCON);
					v &= ~(PCA9564_I2CCON_SI|PCA9564_I2CCON_STA);
					clockport_write(sp, PCA9564_I2CCON, v);
					break;

				case PCA9564_I2CSTA_SLAW_TX_ACK_RX:	 /* 0x18 */
					v = clockport_read(sp, PCA9564_I2CCON);
					v &= ~(PCA9564_I2CCON_SI|PCA9564_I2CCON_STA);
					if ((sp->bytes_count) < sp->buf_size) {
						clockport_write(sp, PCA9564_I2CDAT, sp->buf[sp->bytes_count]);
					} else {
						v |= (PCA9564_I2CCON_STO);
					}
					clockport_write(sp, PCA9564_I2CCON, v);
					if (sp->bytes_count == sp->buf_size) {
						sp->cur_result = RESULT_OK;
						Signal(sp->MainTask, sp->sigmask_intr);
					}
					break;

				case PCA9564_I2CSTA_SLAW_TX_NACK_RX:	/* 0x20 */
					v = clockport_read(sp, PCA9564_I2CCON);
					v |= (PCA9564_I2CCON_STO);
					clockport_write(sp, PCA9564_I2CCON, v);
					sp->cur_result = RESULT_NO_REPLY;
					Signal(sp->MainTask, sp->sigmask_intr);
					break;

				case PCA9564_I2CSTA_DATA_TX_ACK_RX:	/* 0x28 */
					v = clockport_read(sp, PCA9564_I2CCON);
					(sp->bytes_count)++;
					if (sp->bytes_count < sp->buf_size) {
						clockport_write(sp, PCA9564_I2CDAT, sp->buf[sp->bytes_count]);
					} else {
						v |= (PCA9564_I2CCON_STO);
					}
					v &= ~(PCA9564_I2CCON_SI);
					clockport_write(sp, PCA9564_I2CCON, v);
					if (sp->bytes_count == sp->buf_size) {
						sp->cur_result = RESULT_OK;
						Signal(sp->MainTask, sp->sigmask_intr);
					}
					break;

				default:
					clockport_write(sp, PCA9564_I2CCON, 0);
					sp->cur_result = RESULT_HARDW_BUSY;
					Signal(sp->MainTask, sp->sigmask_intr);
					break;
			}
			break;
		case OP_NOP:
			clockport_write(sp, PCA9564_I2CCON, 0);
			sp->cur_result = RESULT_OK; //RESULT_HARDW_BUSY;
			Signal(sp->MainTask, sp->sigmask_intr);
			break;
	}

#ifdef DEBUG
  sp->in_isr = FALSE;
#endif /* DEBUG */
	return 0;
}

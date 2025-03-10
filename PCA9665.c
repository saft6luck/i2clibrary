#include "library.h"
#include "PCA9665.h"


void
pca9665_init(I2C_state_t *sp)
{
	// set I2C to requested clock rate (defined in LibGlobal structure) and activate internal oscilator
	
	/* select bus mode */
	clockport_write_indirect(sp, PCA9665_MODE, sp->PCA_Mode);

	/* low byte of I2C clock rate PCA9665_SCLL */
	clockport_write_indirect(sp, PCA9665_SCLL, sp->PCA_ClockRate_low);

	/* high byte of I2C clock rate PCA9665_SCLH */
	clockport_write_indirect(sp, PCA9665_SCLH, sp->PCA_ClockRate_high);
		
	/* set byte mode and activate clock generator */
	clockport_write(sp, PCA9665_CON,  PCA9665_CON_MODE_BYTE | PCA9665_CON_ENSIO);
}


void
pca9665_exec(I2C_state_t *sp, UBYTE address, ULONG size, UBYTE **buf)
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
pca9665_send_start(I2C_state_t *sp)
{
	UBYTE c;

	c = clockport_read(sp, PCA9665_CON);
	c |= PCA9665_CON_STA|PCA9665_CON_AA;
	clockport_write(sp, PCA9665_CON, c);	/* send START condition */

}

/* Interrupt service routine. */
__saveds int pca9665_isr(I2C_state_t *sp __asm("a1"))
{
	UBYTE v;

#ifdef DEBUG
  	sp->in_isr = TRUE;
	sp->isr_called++;
#endif /* DEBUG */

	/* our interrupt? */
	if (!(clockport_read(sp, PCA9665_CON) & PCA9665_CON_SI)) {

#ifdef DEBUG
		sp->in_isr = FALSE;
#endif /* DEBUG */
		return 1;
	}

	switch (sp->I2C_CurrentOperationMode) {
		case OP_BUFFER_READ:
		case OP_BYTE_READ:
			switch (clockport_read(sp, PCA9665_STA)) {
				/* A START condition has been transmitted */
				case PCA9665_STA_START_SENT:		/* 0x08 */
					/* send SLAVE address next */
					clockport_write(sp, PCA9665_DAT, sp->slave_addr);
					/* clear SI (interrupt) and STA (START) flags */
					v = clockport_read(sp, PCA9665_CON);
					v &= ~(PCA9665_CON_SI|PCA9665_CON_STA);
					clockport_write(sp, PCA9665_CON, v);
					break;

				/* SLA+R has been transmitted; ACK has been received */
				case PCA9665_STA_SLAR_TX_ACK_RX:	/* 0x40 */
					/* clear SI (interrupt) flag */
					v = clockport_read(sp, PCA9665_CON);
					v &= ~(PCA9665_CON_SI);

					/* still space in buffer? */
					if ((sp->bytes_count) <= sp->buf_size)
						v |= (PCA9665_CON_AA);
					else
						v &= ~(PCA9665_CON_AA); /* last byte */
					clockport_write(sp, PCA9665_CON, v);
					break;

				/* SLA+R has been transmitted; NACK has been received */
				case PCA9665_STA_SLAR_TX_NACK_RX:	/* 0x48 */
					/* clear SI (interrupt) flag and set STOP flag */
					v = clockport_read(sp, PCA9665_CON);
					v &= ~(PCA9665_CON_SI);
					v |= (PCA9665_CON_STO);	/* send stop */
					clockport_write(sp, PCA9665_CON, v);

					sp->cur_result = RESULT_NO_REPLY; /* NO ACK */
					Signal(sp->MainTask, sp->sigmask_intr);
					break;

				/* Data byte has been received; ACK has been returned */
				case PCA9665_STA_DATA_RX_ACK_TX:	/* 0x50 */
					/* copy received byte to buffer */
					sp->buf[sp->bytes_count] = clockport_read(sp, PCA9665_DAT);
					(sp->bytes_count)++;
					/* clear SI (interrupt) flag */
					v = clockport_read(sp, PCA9665_CON);
					v &= ~(PCA9665_CON_SI);

					/* still space in buffer? */
					if ((sp->bytes_count+1) < sp->buf_size)
						v |= (PCA9665_CON_AA);
					else
						v &= ~(PCA9665_CON_AA); /* last byte */
					clockport_write(sp, PCA9665_CON, v);
					break;

				/* Data byte has been received; NACK has been returned */
				case PCA9665_STA_DATA_RX_NACK_TX:	/* 0x58 */
					/* copy received byte to buffer */
					sp->buf[sp->bytes_count] = clockport_read(sp, PCA9665_DAT);
					(sp->bytes_count)++;
					
					/* clear SI (interrupt) flag and set STOP flag */
					v = clockport_read(sp, PCA9665_CON);
					v &= ~(PCA9665_CON_SI);
					v |= (PCA9665_CON_AA|PCA9665_CON_STO);	/* send stop */
					clockport_write(sp, PCA9665_CON, v);
					
					sp->cur_result = RESULT_OK;
					Signal(sp->MainTask, sp->sigmask_intr);
					break;

				default: /**/
					/* clear all flags */
					clockport_write(sp, PCA9665_CON, 0);
					sp->cur_result = RESULT_HARDW_BUSY;
					Signal(sp->MainTask, sp->sigmask_intr);
					break;
			}
			break;

		case OP_BUFFER_WRITE:
		case OP_BYTE_WRITE:
			switch (clockport_read(sp, PCA9665_STA)) {
				
				/* A START condition has been transmitted */
				case PCA9665_STA_START_SENT:		/* 0x08 */
					/* send SLAVE address next */
					clockport_write(sp, PCA9665_DAT, sp->slave_addr);
					v = clockport_read(sp, PCA9665_CON);
					v &= ~(PCA9665_CON_SI|PCA9665_CON_STA);
					clockport_write(sp, PCA9665_CON, v);
					break;

				/* SLA+W has been transmitted; ACK has been received */
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

				/* SLA+W has been transmitted; NACK has been received */
				case PCA9665_STA_SLAW_TX_NACK_RX:	/* 0x20 */
					v = clockport_read(sp, PCA9665_CON);
					v |= (PCA9665_CON_STO);
					clockport_write(sp, PCA9665_CON, v);
					sp->cur_result = RESULT_NO_REPLY;
					Signal(sp->MainTask, sp->sigmask_intr);
					break;

				/* Data byte in I2CDAT has been transmitted; ACK has been received */
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

				/* Data byte in I2CDAT has been transmitted; NACK has been received */
				case PCA9665_STA_DATA_TX_NACK_RX:   /* 0x30 */
					/* skip to default */
				default:
					clockport_write(sp, PCA9665_CON, 0);
					sp->cur_result = RESULT_HARDW_BUSY;
					Signal(sp->MainTask, sp->sigmask_intr);
					break;
			}
			break;

		case OP_NOP:
		default:
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

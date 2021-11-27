#include "akuhei2c.h"

void __restore_a4(void)
{
    __asm volatile("\tlea ___a4_init, a4");
}

UBYTE
clockport_read(pca9564_state_t *sp, UBYTE reg)
{
	UBYTE v;
	UBYTE *ptr;

	ptr = sp->cp + (reg * CLOCKPORT_STRIDE);
	v = *ptr;
#ifdef DEBUG
	if (!(sp->in_isr))
		printf("DEBUG: read %x from %p\n", (int) v, (void*) ptr);
#endif /* DEBUG */

	return v;
}

void
clockport_write(pca9564_state_t *sp, UBYTE reg, UBYTE value)
{
	UBYTE *ptr;

	ptr = (sp->cp) + (reg * CLOCKPORT_STRIDE);
#ifdef DEBUG
	if (!(sp->in_isr))
		printf("DEBUG: write %x to %p\n", (int) value, (void*) ptr);
#endif /* DEBUG */

	*ptr = value;
}

void
pca9564_write(pca9564_state_t *sp, UBYTE address, ULONG size, UBYTE **buf)
{
	sp->cur_op = EN_OP_WRITE;
	pca9564_exec(sp, address, size, buf);
	sp->cur_op = EN_OP_NOP;
}

void
pca9564_read(pca9564_state_t *sp, UBYTE address, ULONG size, UBYTE **buf)
{
	sp->cur_op = EN_OP_READ;
	pca9564_exec(sp, address, size, buf);
	sp->cur_op = EN_OP_NOP;
}

void
pca9564_exec(pca9564_state_t *sp, UBYTE address, ULONG size, UBYTE **buf)
{
	sp->slave_addr = address;
	sp->buf_size = size;
	sp->bytes_count = 0;
	sp->buf = *buf;

	pca9564_send_start(sp);

	Wait(sp->sigmask_intr);

/*	if (sp->cur_result != EN_RESULT_OK) {
		printf("OP: failed!\n");
		pca9564_dump_state(sp);
	}*/

	sp->buf_size = 0;
	sp->slave_addr = 0;
}

void
pca9564_send_start(pca9564_state_t *sp)
{
	UBYTE c;

	c = clockport_read(sp, I2CCON);
	c |= I2CCON_STA|I2CCON_AA;
	clockport_write(sp, I2CCON, c);	/* send START condition */

}

/*void
pca9564_dump_state(pca9564_state_t *sp)
{
	UBYTE c, s, d;

	c = clockport_read(sp, I2CCON);
	s = clockport_read(sp, I2CSTA);
	d = clockport_read(sp, I2CDAT);
	printf("I2CCON: %x, I2CSTA: %x, I2CDAT: %x\n", c, s, d);
	switch (s) {
    case I2CSTA_SLAW_TX_ACK_RX:      0x18
			printf("SLAW_TX_ACK_RX\n"); break;
    case I2CSTA_DATA_TX_ACK_RX:      0x28
			printf("DATA_TX_ACK_RX\n"); break;
		case I2CSTA_DATA_TX_NACK_RX:    -0x30-
			printf("DATA_TX_NACK_RX\n"); break;
    case I2CSTA_SLAR_TX_ACK_RX:      0x40
			printf("SLAR_TX_ACK_RX\n"); break;
    case I2CSTA_SLAR_TX_NACK_RX:     0x48
			printf("SLAR_TX_NACK_RX\n"); break;
    case I2CSTA_DATA_RX_ACK_TX:      0x50
			printf("DATA_RX_ACK_TX\n"); break;
    case I2CSTA_DATA_RX_NACK_TX:     0x58
			printf("DATA_RX_NACK_TX\n"); break;
		case I2CSTA_SDA_STUCK:           0x70
			printf("SDA_STUCK\n"); break;
		case I2CSTA_SCL_STUCK:           0x90
			printf("SCL_STUCK\n"); break;
		default: break;
	}
}*/

/* Interrupt service routine. */
__saveds int pca9564_isr(pca9564_state_t *sp __asm("a1"))
{
	UBYTE v;

	sp->in_isr = TRUE;
	sp->isr_called++;

	if (!(clockport_read(sp, I2CCON) & I2CCON_SI)) {
		sp->in_isr = FALSE;
		return 0;
	}

	switch (sp->cur_op) {
	case EN_OP_READ:
		switch (clockport_read(sp, I2CSTA)) {
		case I2CSTA_START_SENT:		/* 0x08 */
		case I2CSTA_REP_START_SENT:		/* 0x10 */
			clockport_write(sp, I2CDAT, (sp->slave_addr << 1) | 1);
			v = clockport_read(sp, I2CCON);
			v &= ~(I2CCON_SI|I2CCON_STA);
			clockport_write(sp, I2CCON, v);
			break;
		case I2CSTA_SLAR_TX_ACK_RX:	/* 0x40 */
			v = clockport_read(sp, I2CCON);
			v &= ~(I2CCON_SI);

			if ((sp->bytes_count+1) < sp->buf_size)
				v |= (I2CCON_AA);
			else
				v &= ~(I2CCON_AA); /* last byte */

			clockport_write(sp, I2CCON, v);
			break;
		case I2CSTA_SLAR_TX_NACK_RX:	/* 0x48 */
			sp->buf[sp->bytes_count] = clockport_read(sp, I2CDAT);
			(sp->bytes_count)++;

			v = clockport_read(sp, I2CCON);
			v &= ~(I2CCON_SI);
			v |= (I2CCON_AA|I2CCON_STO);	/* send stop */

			clockport_write(sp, I2CCON, v);

			sp->cur_result = EN_RESULT_ERR;
			Signal(sp->MainTask, sp->sigmask_intr);
			break;
		case I2CSTA_DATA_RX_ACK_TX:	/* 0x50 */
			sp->buf[sp->bytes_count] = clockport_read(sp, I2CDAT);
			(sp->bytes_count)++;

			v = clockport_read(sp, I2CCON);
			v &= ~(I2CCON_SI);

			if ((sp->bytes_count+1) < sp->buf_size)
				v |= (I2CCON_AA);
			else
				v &= ~(I2CCON_AA); /* last byte */

			clockport_write(sp, I2CCON, v);
			break;
		case I2CSTA_DATA_RX_NACK_TX:	/* 0x58 */
			sp->buf[sp->bytes_count] = clockport_read(sp, I2CDAT);
			(sp->bytes_count)++;

			v = clockport_read(sp, I2CCON);
			v &= ~(I2CCON_SI);
			v |= (I2CCON_AA|I2CCON_STO);	/* send stop */

			clockport_write(sp, I2CCON, v);

			sp->cur_result = EN_RESULT_OK;
			Signal(sp->MainTask, sp->sigmask_intr);
			break;
		default:
			clockport_write(sp, I2CCON, 0);
			sp->cur_result = EN_RESULT_ERR;
			Signal(sp->MainTask, sp->sigmask_intr);
			break;
		}
		break;
	case EN_OP_WRITE:
		switch (clockport_read(sp, I2CSTA)) {
		case I2CSTA_START_SENT:		/* 0x08 */
			clockport_write(sp, I2CDAT, (sp->slave_addr << 1) | 0);
			v = clockport_read(sp, I2CCON);
			v &= ~(I2CCON_SI|I2CCON_STA);
			clockport_write(sp, I2CCON, v);
			break;
		case I2CSTA_SLAW_TX_ACK_RX:	/* 0x18 */
			clockport_write(sp, I2CDAT, sp->buf[sp->bytes_count]);
			v = clockport_read(sp, I2CCON);
			v &= ~(I2CCON_SI);
			clockport_write(sp, I2CCON, v);
			break;
		case I2CSTA_DATA_TX_ACK_RX:	/* 0x28 */
		/*case I2CSTA_DATA_TX_NACK_RX:*/    /* 0x30 */
			v = clockport_read(sp, I2CCON);

			(sp->bytes_count)++;
			if (sp->bytes_count < sp->buf_size) {
				clockport_write(sp, I2CDAT, sp->buf[sp->bytes_count]);
			} else {
				v |= (I2CCON_STO);
			}

			v &= ~(I2CCON_SI);
			clockport_write(sp, I2CCON, v);

			if (sp->bytes_count == sp->buf_size) {
				sp->cur_result = EN_RESULT_OK;
				Signal(sp->MainTask, sp->sigmask_intr);
			}

			break;
		default:
			clockport_write(sp, I2CCON, 0);
			sp->cur_result = EN_RESULT_ERR;
			Signal(sp->MainTask, sp->sigmask_intr);
			break;
		}
		break;
	case EN_OP_NOP:
		clockport_write(sp, I2CCON, 0);
		sp->cur_result = EN_RESULT_ERR;
		Signal(sp->MainTask, sp->sigmask_intr);
		break;
	}

	sp->in_isr = FALSE;
	return 0;
}

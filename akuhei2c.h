#ifndef AKUHEI2C_H
#define AKUHEI2C_H

#include <stdio.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/interrupts.h>

#include <hardware/intbits.h>

#ifdef DEBUG
//#include <clib/debug_protos.h>
#include "debug.h"
#endif /* DBG */

#define CLOCKPORT_BASE		0xD80001
#define CLOCKPORT_STRIDE        2

#define I2C_PCA_STA		0x00 /* STATUS  Read Only  */
#define I2C_PCA_TO		0x00 /* TIMEOUT Write Only */
#define I2C_PCA_DAT		0x01 /* DATA    Read/Write */
#define I2C_PCA_ADR		0x02 /* OWN ADR Read/Write */
#define I2C_PCA_CON		0x03 /* CONTROL Read/Write */

#define I2CSTA                  0
#define I2CTO                   0
#define I2CDAT                  1
#define I2CADR                  2
#define I2CCON                  3

#define I2CCON_CR0              (1 << 0)
#define I2CCON_CR1              (1 << 1)
#define I2CCON_CR2              (1 << 2)
#define I2CCON_CR_330KHZ        (0x0)
#define I2CCON_CR_288KHZ        (0x1)
#define I2CCON_CR_217KHZ        (0x2)
#define I2CCON_CR_146KHZ        (0x3)
#define I2CCON_CR_88KHZ         (0x4)
#define I2CCON_CR_59KHZ         (0x5)
#define I2CCON_CR_MASK          (0x7)
#define I2CCON_SI               (1 << 3)
#define I2CCON_STO              (1 << 4)
#define I2CCON_STA              (1 << 5)
#define I2CCON_ENSIO            (1 << 6)
#define I2CCON_AA               (1 << 7)

#define I2C_PCA_CON_AA		0x80 /* Assert Acknowledge */
#define I2C_PCA_CON_ENSIO	0x40 /* Enable */
#define I2C_PCA_CON_STA		0x20 /* Start */
#define I2C_PCA_CON_STO		0x10 /* Stop */
#define I2C_PCA_CON_SI		0x08 /* Serial Interrupt */
#define I2C_PCA_CON_CR		0x07 /* Clock Rate (MASK) */

#define I2CSTA_START_SENT       0x08
#define I2CSTA_REP_START_SENT   0x10

#define I2CSTA_SLAW_TX_ACK_RX   0x18
#define I2CSTA_SLAW_TX_NACK_RX  0x20
#define I2CSTA_DATA_TX_ACK_RX   0x28
#define I2CSTA_DATA_TX_NACK_RX  0x30

#define I2CSTA_SLAR_TX_ACK_RX   0x40
#define I2CSTA_SLAR_TX_NACK_RX  0x48
#define I2CSTA_DATA_RX_ACK_TX   0x50
#define I2CSTA_DATA_RX_NACK_TX  0x58

#define I2CSTA_IDLE             0xF8
#define I2CSTA_SDA_STUCK        0x70
#define I2CSTA_SCL_STUCK        0x90
#define I2CSTA_ARB_LOST         0x38
#define I2CSTA_ILLEGAL_COND     0x00

typedef enum {
        OP_NOP,
        OP_READ,
        OP_WRITE
} op_t;

typedef enum {
        RESULT_OK=0,        /* Last send/receive was OK */
        RESULT_REJECT=1,       /* Data not acknowledged (i.e. unwanted) */
        RESULT_NO_REPLY=2,      /* Chip address apparently invalid */
        RESULT_SDA_TRASHED=3,
        RESULT_SDA_LO=4,   /* SDA always LO \_wrong interface attached, */
        RESULT_SDA_HI=5,
        RESULT_SCL_TIMEOUT=6,
        RESULT_SCL_HI=7,
        RESULT_HARDW_BUSY=8
} result_t;

/* glorious god object that holds the state of everything in this program; tldr */
typedef struct {
        UBYTE cur_op;
        UBYTE cur_result;

        UBYTE *cp;
        UBYTE stride;

        BYTE sig_intr;
        LONG sigmask_intr;
        struct Task *MainTask;

        UBYTE *buf;
        ULONG buf_size;
        ULONG bytes_count;

        UBYTE slave_addr;
/*#ifdef DEBUG*/
        int isr_called; /* how may times ISR was called */
        BOOL in_isr;
/*#endif  DEBUG */
} pca9564_state_t;

UBYTE clockport_read(pca9564_state_t *, UBYTE);
void clockport_write(pca9564_state_t *, UBYTE, UBYTE);
__saveds int pca9564_isr(pca9564_state_t * __asm("a1"));
/*void pca9564_dump_state(pca9564_state_t *);*/
void pca9564_send_start(pca9564_state_t *);
void pca9564_read(pca9564_state_t *, UBYTE, ULONG, UBYTE **);
void pca9564_write(pca9564_state_t *, UBYTE, ULONG, UBYTE **);
void pca9564_exec(pca9564_state_t *, UBYTE, ULONG, UBYTE **);

#endif      /* AKUHEI2C_H */

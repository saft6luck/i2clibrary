#pragma once

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

// direct access
#define PCA9665_STA		0x00 /* I2CSTA – status register                        – read only  */
#define PCA9665_INDPTR		0x00 /* INDPTR – indirect address pointer               – write only */
#define PCA9665_DAT		0x01 /* I2CDAT – data register  (up to 68 bytes buffer) – read/write */
#define PCA9665_INDIRECT	0x02 /* INDIRECT – indirect register access             – read/write */
#define PCA9665_CON		0x03 /* I2CCON – control register                       – read/write */

// indirect access
#define PCA9665_COUNT		0x00 /* I2CCOUNT – byte count – read/write  */
#define PCA9665_ADR		0x01 /* I2CADR – own address – read/write */
#define PCA9665_SCLL		0x02 /* I2CSCLL – SCL LOW period – read/write */
#define PCA9665_SCLH		0x03 /* I2CSCLH – SCL HIGH period – read/write */
#define PCA9665_TO		0x04 /* I2CTO – TIMEOUT register – read/write */
#define PCA9665_PRESET		0x05 /* I2CPRESET – software reset register – write only */
#define PCA9665_MODE		0x06 /* I2CMODE – I2C-bus mode register – read/write */

// 
#define PCA9665_CON_SI               (1 << 3)
#define PCA9665_CON_STO              (1 << 4)
#define PCA9665_CON_STA              (1 << 5)
#define PCA9665_CON_ENSIO            (1 << 6)
#define PCA9665_CON_AA               (1 << 7)

#define PCA9665_CON_AA		0x80 /* Assert Acknowledge */
#define PCA9665_CON_ENSIO	0x40 /* Enable */
#define PCA9665_CON_STA		0x20 /* Start */
#define PCA9665_CON_STO		0x10 /* Stop */
#define PCA9665_CON_SI		0x08 /* Serial Interrupt */
#define PCA9665_CON_MODE	0x01 /* Mode (MASK) */

#define PCA9665_CON_MODE_BYTE	0x00 /* BYTE Mode */
#define PCA9665_CON_MODE_BUFFER	0x01 /* BUFFER Mode (up to 68 bytes) */


#define PCA9665_STA_START_SENT       0x08
#define PCA9665_STA_REP_START_SENT   0x10

#define PCA9665_STA_SLAW_TX_ACK_RX   0x18
#define PCA9665_STA_SLAW_TX_NACK_RX  0x20
#define PCA9665_STA_DATA_TX_ACK_RX   0x28
#define PCA9665_STA_DATA_TX_NACK_RX  0x30

#define PCA9665_STA_SLAR_TX_ACK_RX   0x40
#define PCA9665_STA_SLAR_TX_NACK_RX  0x48
#define PCA9665_STA_DATA_RX_ACK_TX   0x50
#define PCA9665_STA_DATA_RX_NACK_TX  0x58

#define PCA9665_STA_IDLE             0xF8
#define PCA9665_STA_SDA_STUCK        0x70
#define PCA9665_STA_SCL_STUCK        0x90
#define PCA9665_STA_ARB_LOST         0x38
#define PCA9665_STA_ILLEGAL_COND     0x00


#define PCA9665_CON_CR_330KHZ        (0x0)
#define PCA9665_CON_CR_288KHZ        (0x1)
#define PCA9665_CON_CR_217KHZ        (0x2)
#define PCA9665_CON_CR_146KHZ        (0x3)
#define PCA9665_CON_CR_88KHZ         (0x4)
#define PCA9665_CON_CR_59KHZ         (0x5)
#define PCA9665_CON_CR_MASK          (0x7)

#define PCA9665_ADR_DEFAULT          0xE0



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

typedef enum {
        PCA_UNKNOWN,
        PCA_9564,
        PCA_9665
} PCA_TYPE_t;


/* glorious god object that holds the state of everything in this program; tldr */
typedef struct {
        UBYTE cur_op;
        UBYTE cur_result;

        UBYTE *cp;
        UBYTE stride;
        UBYTE cr;

        BYTE sig_intr;
        LONG sigmask_intr;
        struct Task *MainTask;

        UBYTE *buf;
        ULONG buf_size;
        ULONG bytes_count;

        UBYTE slave_addr;
        PCA_TYPE_t pca_type;
/*#ifdef DEBUG*/
        int isr_called; /* how may times ISR was called */
        BOOL in_isr;
/*#endif  DEBUG */
} pca_state_t;

UBYTE clockport_read(pca_state_t *, UBYTE);
void clockport_write(pca_state_t *, UBYTE, UBYTE);
__saveds int pca9665_isr(pca_state_t * __asm("a1"));
/*void pca9665_dump_state(pca_state_t *);*/
void pca9665_send_start(pca_state_t *);
void pca9665_read(pca_state_t *, UBYTE, ULONG, UBYTE **);
void pca9665_write(pca_state_t *, UBYTE, ULONG, UBYTE **);
void pca9665_exec(pca_state_t *, UBYTE, ULONG, UBYTE **);

/* PCA9665_H */

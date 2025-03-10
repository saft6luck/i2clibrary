#pragma once

#include <stdio.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/interrupts.h>

#include <hardware/intbits.h>

#include "library_common.h"

#ifdef DEBUG
//#include <clib/debug_protos.h>
#include "debug.h"
#endif /* DBG */

#define PCA9564_I2C_PCA_STA		0x00 /* STATUS  Read Only  */
#define PCA9564_I2C_PCA_TO		0x00 /* TIMEOUT Write Only */
#define PCA9564_I2C_PCA_DAT		0x01 /* DATA    Read/Write */
#define PCA9564_I2C_PCA_ADR		0x02 /* OWN ADR Read/Write */
#define PCA9564_I2C_PCA_CON		0x03 /* CONTROL Read/Write */

#define PCA9564_I2CSTA                  0
#define PCA9564_I2CTO                   0
#define PCA9564_I2CDAT                  1
#define PCA9564_I2CADR                  2
#define PCA9564_I2CCON                  3

#define PCA9564_I2CCON_CR0              (1 << 0)
#define PCA9564_I2CCON_CR1              (1 << 1)
#define PCA9564_I2CCON_CR2              (1 << 2)
#define PCA9564_I2CCON_CR_330KHZ        (0x0)
#define PCA9564_I2CCON_CR_288KHZ        (0x1)
#define PCA9564_I2CCON_CR_217KHZ        (0x2)
#define PCA9564_I2CCON_CR_146KHZ        (0x3)
#define PCA9564_I2CCON_CR_88KHZ         (0x4)
#define PCA9564_I2CCON_CR_59KHZ         (0x5)
#define PCA9564_I2CCON_CR_MASK          (0x7)
#define PCA9564_I2CCON_SI               (1 << 3)
#define PCA9564_I2CCON_STO              (1 << 4)
#define PCA9564_I2CCON_STA              (1 << 5)
#define PCA9564_I2CCON_ENSIO            (1 << 6)
#define PCA9564_I2CCON_AA               (1 << 7)

#define PCA9564_I2C_PCA_CON_AA		0x80 /* Assert Acknowledge */
#define PCA9564_I2C_PCA_CON_ENSIO	0x40 /* Enable */
#define PCA9564_I2C_PCA_CON_STA		0x20 /* Start */
#define PCA9564_I2C_PCA_CON_STO		0x10 /* Stop */
#define PCA9564_I2C_PCA_CON_SI		0x08 /* Serial Interrupt */
#define PCA9564_I2C_PCA_CON_CR		0x07 /* Clock Rate (MASK) */

#define PCA9564_I2CSTA_START_SENT       0x08
#define PCA9564_I2CSTA_REP_START_SENT   0x10

#define PCA9564_I2CSTA_SLAW_TX_ACK_RX   0x18
#define PCA9564_I2CSTA_SLAW_TX_NACK_RX  0x20
#define PCA9564_I2CSTA_DATA_TX_ACK_RX   0x28
#define PCA9564_I2CSTA_DATA_TX_NACK_RX  0x30

#define PCA9564_I2CSTA_SLAR_TX_ACK_RX   0x40
#define PCA9564_I2CSTA_SLAR_TX_NACK_RX  0x48
#define PCA9564_I2CSTA_DATA_RX_ACK_TX   0x50
#define PCA9564_I2CSTA_DATA_RX_NACK_TX  0x58

#define PCA9564_I2CSTA_IDLE             0xF8
#define PCA9564_I2CSTA_SDA_STUCK        0x70
#define PCA9564_I2CSTA_SCL_STUCK        0x90
#define PCA9564_I2CSTA_ARB_LOST         0x38
#define PCA9564_I2CSTA_ILLEGAL_COND     0x00


__saveds int pca9564_isr(I2C_state_t * __asm("a1"));

void pca9564_init(I2C_state_t *);
void pca9564_send_start(I2C_state_t *);
void pca9564_exec(I2C_state_t *, UBYTE, ULONG, UBYTE **);



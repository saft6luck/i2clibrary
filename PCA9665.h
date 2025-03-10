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


// PCA9665
// direct access registers - 4 registers, 1 is different for read or write (see comment)
#define PCA9665_STA		   0x00 /* I2CSTA – status register                        – read only  */
#define PCA9665_INDPTR  	0x00 /* INDPTR – indirect address pointer               – write only */
#define PCA9665_DAT		   0x01 /* I2CDAT – data register  (up to 68 bytes buffer) – read/write */
#define PCA9665_INDIRECT	0x02 /* INDIRECT – indirect register access             – read/write */
#define PCA9665_CON		   0x03 /* I2CCON – control register                       – read/write */

// INDPTR
// PCA9665 indirect access registers - 7 registers
#define PCA9665_COUNT		0x00 /* I2CCOUNT – byte count – read/write  */
#define PCA9665_ADR		   0x01 /* I2CADR – own address – read/write */
#define PCA9665_SCLL		   0x02 /* I2CSCLL – SCL LOW period – read/write */
#define PCA9665_SCLH		   0x03 /* I2CSCLH – SCL HIGH period – read/write */
#define PCA9665_TO		   0x04 /* I2CTO – TIMEOUT register – read/write */
#define PCA9665_PRESET  	0x05 /* I2CPRESET – software reset register – write only */
#define PCA9665_MODE		   0x06 /* I2CMODE – I2C-bus mode register – read/write */

#define PCA9665_INDPTR_MASK     0x07 /* Mask to access INDPTR register -> only bit 0 - 2 are allowed */

// I2CCON
// bit definition
#define PCA9665_CON_AA               (1 << 7) /* Assert Acknowledge */
#define PCA9665_CON_ENSIO            (1 << 6) /* Enable */
#define PCA9665_CON_STA              (1 << 5) /* Start */
#define PCA9665_CON_STO              (1 << 4) /* Stop */
#define PCA9665_CON_SI               (1 << 3) /* Serial Interrupt */

#define IS_ACTIVE( a, bit )        ((a & bit##_MASK) != 0)

#define PCA9665_CON_AA_MASK	   0x80 /* Assert Acknowledge */
#define PCA9665_CON_ENSIO_MASK	0x40 /* Enable */
#define PCA9665_CON_STA_MASK	   0x20 /* Start */
#define PCA9665_CON_STO_MASK	   0x10 /* Stop */
#define PCA9665_CON_SI_MASK	   0x08 /* Serial Interrupt */
#define PCA9665_CON_MODE_MASK	   0x01 /* Mode (MASK) */

#define PCA9665_CON_MODE_BYTE	   0x00 /* BYTE Mode */
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

/* I2C controller status values */
#define PCA9665_STA_IDLE                        0xF8
#define PCA9665_STA_SDA_STUCK                   0x70
#define PCA9665_STA_SCL_STUCK                   0x90
#define PCA9665_STA_ARB_LOST                    0x38
#define PCA9665_STA_ILLEGAL_COND                0x00
#define PCA9665_STA_START_TRANSMITTED           0x08 /* A START condition has been transmitted */
#define PCA9665_STA_REPEATED_START_TRANSMITTED  0x10 /* A repeated START condition has been transmitted */

/* Master Transmitter */
#define PCA9665_STA_SLA_W_ACK                   0x18 /* SLA+W has been transmitted; ACK has been received */
#define PCA9665_STA_SLA_W_NACK                  0x20 /* SLA+W has been transmitted; NACK has been received */
#define PCA9665_STA_DATA_W_ACK                  0x28 /* Byte mode (MODE = 0): Data byte in I2CDAT has been transmitted; ACK has been received */
                                                     /* Buffered mode (MODE = 1): BC[6:0] bytes in I2CDAT have been transmitted; ACK has been received for all of them */
#define PCA9665_STA_DATA_W_NACK                 0x30 /* Byte mode (MODE = 0): Data byte in I2CDAT has been transmitted; NACK has been received */
                                                     /* Buffered mode (MODE = 1): Up to BC[6:0] bytes in I2CDAT have been transmitted; NACK has been received for the last byte */

/* Master Receiver */
#define PCA9665_STA_SLA_R_ACK                   0x40 /* SLA+R has been transmitted; ACK has been received */
#define PCA9665_STA_SLA_R_NACK                  0x48 /* SLA+R has been transmitted; NACK has been received */
#define PCA9665_STA_DATA_R_ACK                  0x50 /* Byte mode (MODE = 0): Data byte has been received; ACK has been returned */
                                                     /* Buffered mode (MODE = 1): BC[6:0] data bytes have been received; ACK has been returned for all the bytes */
#define PCA9665_STA_DATA_R_NACK                 0x58 /* Byte mode (MODE = 0): Data byte has been received; NACK has been returned */
                                                     /* Buffered mode (MODE = 1): BC[6:0] data bytes have been received; ACK has been returned for all the bytes, except for the last one where NACK bit has been returned */

/* I2CSCLL / I2CSCLH / I2CMODE values */
/* 
 Minimum speed values per mode => maximun possible speed per mode 
 I2CSCLL  I2CSCLH  I2C-bus frequency (kHz)   AC[1:0] Mode
  (hex)    (hex)   PCA9665[2] PCA9665A[3]
   9D       86       98.0        103.3       00 Standard
   2C       14      371.1        371.4       01 Fast
   11       09      836.8        788.6       10 Fast-mode Plus
   0E       05      1015         932.8       11 Turbo mode
*/

#define PCA9665_SCLL_CR_370KHZ        (0x2C)
#define PCA9665_SCLH_CR_370KHZ        (0x14)
#define PCA9665_MODE_CR_370KHZ        (0x01)    /* Fast Mode*/

#define PCA9665_SCLL_CR_100KHZ_LOW    (0x9D)
#define PCA9665_SCLH_CR_100KHZ_HIGH   (0x86)
#define PCA9665_MODE_CR_100KHZ_MODE   (0x00)     /* Standard */

#define PCA9665_MODE_MASK             (0x3)

#define PCA9665_ADR_DEFAULT          0xE0





#define IS_PCA9665(a)             (a == PCA_9665)



__saveds int pca9665_isr(I2C_state_t * __asm("a1"));

void pca9665_init(I2C_state_t *);
void pca9665_send_start(I2C_state_t *);
void pca9665_exec(I2C_state_t *, UBYTE, ULONG, UBYTE **);

/* PCA9665_H */

/*
   i2c.library common header file.
*/

#pragma once

#include <stdio.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/interrupts.h>

#include <hardware/intbits.h>


typedef enum {
        PCA_UNKNOWN,
        PCA_9564,
        PCA_9665
} PCA_TYPE_t;

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
        op_t I2C_CurrentOperationMode;
        result_t cur_result;

        UBYTE *CP_Address;
        UBYTE CP_StepSize;
        UBYTE PCA_ClockRate_low;
        UBYTE PCA_ClockRate_high;
        UBYTE PCA_Mode;

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
} I2C_state_t;

void __restore_a4(void);
UBYTE clockport_read(I2C_state_t *, UBYTE);
void clockport_write(I2C_state_t *, UBYTE, UBYTE);

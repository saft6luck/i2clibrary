/*
   i2c.library internal library definitions.
   Generated with LibMaker 0.12.
*/

#ifndef I2C_LIBRARY_H
#define I2C_LIBRARY_H

#include <exec/libraries.h>
#include <exec/semaphores.h>

#define DBG                     1

#define CLOCKPORT_BASE          (UBYTE *)0xD80001
#define CLOCKPORT_STRIDE        4

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

#define I2CSTA_START_SENT       0x08

#define I2CSTA_SLAR_TX_ACK_RX   0x40
#define I2CSTA_SLAR_TX_NACK_RX  0x48
#define I2CSTA_DATA_RX_ACK_TX   0x50
#define I2CSTA_DATA_RX_NACK_TX  0x58

#define I2CSTA_SLAW_TX_ACK_RX   0x18
#define I2CSTA_SLAW_TX_NACK_RX  0x20
#define I2CSTA_DATA_TX_ACK_RX   0x28
#define I2CSTA_DATA_TX_NACK_RX  0x30

#define I2CSTA_IDLE             0xF8
#define I2CSTA_SDA_STUCK        0x70
#define I2CSTA_SCL_STUCK        0x90

typedef enum {
        OP_NOP,
        OP_READ,
        OP_WRITE
} op_t;

typedef enum {
        RESULT_OK,
        RESULT_ERR
} result_t;

/* glorious god object that holds the state of everything in this program; tldr */
typedef struct {
        op_t cur_op;
        result_t cur_result;

        UBYTE *cp;

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

struct MyLibBase
{
	struct Library          LibNode;
	APTR                    Seglist;
	struct SignalSemaphore  BaseLock;
	BOOL                    InitFlag;

	ULONG                   first_added_field;
	ULONG                   initialized_magic;
	ULONG                   open_magic;

	ULONG                   bringback_magic;
	ULONG                   errortext_magic;
	ULONG                   shutdown_magic;
	ULONG                   getopponent_magic;
	ULONG                   receive_magic;
	ULONG                   send_magic;
	ULONG                   init_magic;
	ULONG                   setdelay_magic;
	ULONG                   free_magic;
	ULONG                   freeresources_magic;
	ULONG                   alloc_magic;

	pca9564_state_t         sc;
  struct Interrupt        *int6;
};

#endif      /* I2C_LIBRARY_H */

#ifndef CLIB_I2C_PROTOS_H
#define CLIB_I2C_PROTOS_H

/*
   i2c.library C prototypes
*/


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

ULONG SetI2CDelay(ULONG);
ULONG SendI2C(UBYTE, UWORD, UBYTE*);
ULONG ReceiveI2C(UBYTE, UWORD, UBYTE*);
STRPTR GetI2COpponent(VOID);
STRPTR I2CErrText(ULONG);
void ShutDownI2C(VOID);
BYTE BringBackI2C(VOID);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CLIB_I2C_PROTOS_H */

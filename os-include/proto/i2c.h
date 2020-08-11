#ifndef _PROTO_I2C_H
#define _PROTO_I2C_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#if !defined(CLIB_I2C_PROTOS_H) && !defined(__GNUC__)
#include <clib/i2c_protos.h>
#endif

#ifndef LIBRARIES_I2C_H
#include <libraries/i2c.h>
#endif

#ifndef __NOLIBBASE__
extern struct Library *I2C_Base;
#endif

#ifdef __GNUC__
#include <inline/i2c.h>
#elif !defined(__VBCC__)
#include <pragma/i2c_lib.h>
#endif

#endif /* _PROTO_I2C_H */

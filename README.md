# i2clibrary

i2c.library implements API v40 of Brian Ipsen and Wilhelm Noeker (https://aminet.net/package/docs/hard/i2clib40)

I2C master utilizes PCA9564 chip connected to clock-port in Amiga 1200, as well as various different alternative ports in expanders and clock ports on Zorro cards.

During initialization, it uses the following base addresses to locate PCA9564. Read-only method of detection is used. The STA register is expected to return I2CSTA_IDLE, ADR and DAT is expected to carry all NULLs.
* 0x00D80001​
* 0x00D84001​
* 0x00D88001​
* 0x00D8C001​
* 0x00D90001 (Fast Port)​
* 0x00D80002 (Gayle adapter)​
Clock ports found on various Zorro cards are also recognized (under development):
* Prisma Megamix​
* Icomp X-Surfer​
* Icomp VarIO​
* Icomp Kickflash​
* Icomp ISDN Surfer​

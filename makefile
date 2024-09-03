# Makefile for i2c.library.
# Generated with LibMaker 0.12.

#PREFIX = /opt/gnu-6.5.0b/bin/m68k-amigaos-gcc/
CC = $(PREFIX)m68k-amigaos-gcc
VASM = vasmm68k_mot
INCLUDE = /opt/amigaos/m68k-amigaos/ndk-include/
VASMFLAGS = -Fhunk

CFLAGS += -s \
 -O2 \
 -noixemul \
 -nostdlib

CFLAGS += -W \
 -Wall \
 -fbaserel \
 -Wpointer-arith

CFLAGS += -Ios-include/

#CFLAGS += -DDEBUG

LD = $(PREFIX)m68k-amigaos-gcc
LDFLAGS = -nostartfiles \
-fbaserel \
-nostdlib \
-noixemul

#STRIP = $(PREFIX)m68k-amigaos-strip \
#STRIP = /opt/gnu-6.5.0b/bin/m68k-amigaos-strip \
#STRIP = m68k-amigaos-strip

STRIP = m68k-amigaos-strip \
 --verbose \
 --strip-unneeded \
 --remove-section .comment

#DBG = -DDEBUG -ldebug

OUTPUT = i2c.library
OBJS = dummy.o library.o

FUNCOBJS = f_alloci2c.o \
 f_freei2c.o \
 f_seti2cdelay.o \
 f_initi2c.o \
 f_sendi2c.o \
 f_receivei2c.o \
 f_geti2copponent.o \
 f_i2cerrtext.o \
 f_shutdowni2c.o \
 f_bringbacki2c.o \
 library_common.o \
 PCA9564.o \
 PCA9665.o
# kprintf.o \

LIBS = -lamiga
LIBS += -lc -lnix
#LIBS += -ldebug

.PHONY: all clean install

all: $(OUTPUT)
	@ls -l $<

clean:
	-rm -rf $(OBJS) $(FUNCOBJS) $(METHOBJS) *.bak *.s *.db $(OUTPUT)

install: all
	cp $(OUTPUT) /SYS/Libs
	-flushlib $(OUTPUT)

kprintf.o: kprintf.asm $(DEPS)
	$(VASM) $(VASMFLAGS) -o $@ $<

$(FUNCOBJS): %.o: %.c library.h
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

$(METHOBJS): %.o: %.c library.h
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

$(OUTPUT).db: $(OBJS) $(FUNCOBJS) $(METHOBJS)
	@echo "Linking $@..."
	$(LD) $(LDFLAGS) $(OBJS) $(FUNCOBJS) $(METHOBJS) $(LIBS) -o $(OUTPUT).db

$(OUTPUT): $(OUTPUT).db
	@echo "Stripping $<..."
	@echo "$(STRIP) -o $(OUTPUT) $(OUTPUT).db"
	@$(STRIP) -o $(OUTPUT) $(OUTPUT).db

library.o: library.c library.h lib_version.h
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

dummy.o: dummy.c lib_version.h
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

f_alloci2c.o: f_alloci2c.c library.h
f_freei2c.o: f_freei2c.c library.h
f_seti2cdelay.o: f_seti2cdelay.c library.h
f_initi2c.o: f_initi2c.c library.h
f_sendi2c.o: f_sendi2c.c library.h
f_receivei2c.o: f_receivei2c.c library.h
f_geti2copponent.o: f_geti2copponent.c library.h
f_i2cerrtext.o: f_i2cerrtext.c library.h
f_shutdowni2c.o: f_shutdowni2c.c library.h
f_bringbacki2c.o: f_bringbacki2c.c library.h

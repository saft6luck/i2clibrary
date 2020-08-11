# Makefile for i2c.library.
# Generated with LibMaker 0.12.

CC = m68k-amigaos-gcc
//CC = /opt/gnu-6.5.0b/bin/m68k-amigaos-gcc
CFLAGS += -s -O2 -noixemul -nostdlib
CFLAGS += -W -Wall -Wpointer-arith
CFLAGS += -Ios-include/
LD = m68k-amigaos-gcc
LDFLAGS = -nostartfiles -nostdlib -noixemul
STRIP = m68k-amigaos-strip --strip-unneeded --remove-section .comment
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
 f_bringbacki2c.o

.PHONY: all clean install

all: $(OUTPUT)
	@ls -l $<

clean:
	-rm -rf $(OBJS) $(FUNCOBJS) $(METHOBJS) *.bak *.s *.db $(OUTPUT)

install: all
	cp $(OUTPUT) /SYS/Libs
	-flushlib $(OUTPUT)

$(FUNCOBJS): %.o: %.c library.h
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

$(METHOBJS): %.o: %.c library.h
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

$(OUTPUT).db: $(OBJS) $(FUNCOBJS) $(METHOBJS)
	@echo "Linking $@..."
	@$(LD) $(LDFLAGS) $(OBJS) $(FUNCOBJS) $(METHOBJS) $(LIBS) -o $(OUTPUT).db

$(OUTPUT): $(OUTPUT).db
	@echo "Stripping $<..."
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

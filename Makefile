# bzzd - a smarter drunkard's walk
# see LICENSE file for copyright and license details.

# bzzd version
VERSION = 0.1

# includes and libs
INCS = -I. -I/usr/include
LIBS = -L/usr/lib -lc -lm

# flags
CFLAGS = -DVERSION=\"${VERSION}\" -std=c99 -pedantic -Wall -Os ${INCS}
LDFLAGS = ${LIBS} -Wl,-rpath .

# compiler and linker
CC = cc

BZZD_SRC = bzzd.c
BZZD_OBJ = ${BZZD_SRC:.c=.o}

all: options ex1 ex2

options:
	@echo bzzd build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

ex1: ${BZZD_OBJ} ex1.o
	@echo CC -o $@
	@${CC} -o $@ ${BZZD_OBJ} ex1.o ${LDFLAGS}

ex2: ${BZZD_OBJ} ex2.o
	@echo CC -o $@
	@${CC} -o $@ ${BZZD_OBJ} ex2.o ${LDFLAGS} -lBearLibTerminal

clean:
	@echo cleaning
	@rm -f *.o ex1

.PHONY: all options clean

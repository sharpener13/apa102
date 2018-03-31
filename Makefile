#
# Defines
#
LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:.
CC=gcc
CFLAGS=-std=c99 -Wall -pedantic -O3
#CFLAGS=-std=c99 -Wall -pedantic -O0 -g -D DEBUG
RM=rm -f
SPECIALS=-D _POSIX_C_SOURCE=200809L -D _DEFAULT_SOURCE
EXES=apa102_test switch_all_on switch_all_off display_test


.EXPORT_ALL_VARIABLES:

#
# General rules
#
all: $(EXES)

clean_o:
	$(RM) *.o

clean_so:
	$(RM) *.so

clean: clean_o clean_so
	$(RM) $(EXES) test

#
# Executable rules
#
apa102_test: apa102_test.spc.o colors.o larson.o libapa102.so
	$(CC) -o $@ $^ $(CFLAGS) -L . -lapa102 -lapa102spi -lm

switch_all_on: switch_all_on.spc.o libapa102.so
	$(CC) -o $@ $^ $(CFLAGS) -L. -lapa102 -lapa102spi

switch_all_off: switch_all_off.spc.o libapa102.so
	$(CC) -o $@ $^ $(CFLAGS) -L . -lapa102

display_test: display_test.spc.o display.o libapa102.so
	$(CC) -o $@ $^ $(CFLAGS) -L . -lapa102 -lm

test: test.o libapa102spi.so
	$(CC) -o $@ $^ $(CFLAGS) -L . -lapa102spi

#
# Library rules
#
libapa102spi.so: apa102spi.pic.o
	$(CC) -o $@ $^ -shared

libapa102.so: apa102.pic.o fifo.pic.o sync_fifo.pic.o debug.pic.o libapa102spi.so
	$(CC) -o $@ $^ -shared -L . -lapa102spi -lpthread

#
# Building block rules
#
%.spc.o: %.c
	$(CC) -c -o $@ $^ $(CFLAGS) $(SPECIALS)

%.pic.o: %.c
	$(CC) -c -o $@ $^ $(CFLAGS) -fpic

%.o: %.c
	$(CC) -c -o $@ $^ $(CFLAGS)


CC=clang
CFLAGS=-std=c99 -Wall -pedantic -O3
RM=rm -f

all: apa102_test

apa102_test: apa102_test.o colors.o larson.o fifo.o sync_fifo.o libapa102.so libapa102spi.so
	$(CC) -o $@ $^ $(CFLAGS) -L . -lapa102 -lapa102spi -lpthread -lm

lib%.so: %.o
	$(CC) -o $@ $^ -shared

apa102_test.o: apa102_test.c
	$(CC) -c -o $@ $^ $(CFLAGS) -D _POSIX_C_SOURCE=200809L

apa102.o: apa102.c
	$(CC) -c -o $@ $^ $(CFLAGS) -fpic

apa102spi.o: apa102spi.c
	$(CC) -c -o $@ $^ $(CFLAGS) -fpic

%.o: %.c
	$(CC) -c -o $@ $^ $(CFLAGS)

clean:
	$(RM) *.o *.so apa102_test

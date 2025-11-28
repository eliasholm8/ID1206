CC = gcc
CFLAGS = -O0 -lpthread --std=c17

BINARIES = q8

.PHONY: all clean

all: $(BINARIES)

q8: q8.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(BINARIES)

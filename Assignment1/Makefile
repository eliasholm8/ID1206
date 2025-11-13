CC = gcc
CFLAGS = -O0

BINARIES = q7 q8

.PHONY: all clean

all: $(BINARIES)

q7: q7.c
	$(CC) $(CFLAGS) -o $@ $<

q8: q8.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(BINARIES)

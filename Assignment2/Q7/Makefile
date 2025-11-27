CC = gcc
CFLAGS = -O0

BINARIES = q7

.PHONY: all clean

all: $(BINARIES)

q7: q7.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(BINARIES)

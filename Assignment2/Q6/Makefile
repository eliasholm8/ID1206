CC = gcc
CFLAGS = -O0 -lpthread

BINARIES = q6

.PHONY: all clean

all: $(BINARIES)

q6: q6.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(BINARIES)

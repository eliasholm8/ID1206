CC = gcc
CFLAGS = -O0

BINARIES = q8

.PHONY: all clean

all: $(BINARIES)
	dd if=/dev/zero of=file_to_map.txt bs=1M count=1

q8: q8.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(BINARIES)

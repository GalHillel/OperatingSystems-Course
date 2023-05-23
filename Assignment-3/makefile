CC = gcc
CFLAGS = -std=c11 -fPIC -D_GNU_SOURCE -Wno-deprecated-declarations
LDFLAGS = -lssl -lcrypto

.PHONY: all clean

all: stnc

stnc: stnc.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f *.txt stnc

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11
LIBS = -lm

all: cmp copy encode decode

cmp: cmp.o
	$(CC) $(CFLAGS) cmp.o -o cmp $(LIBS)

copy: copy.o
	$(CC) $(CFLAGS) copy.o -o copy $(LIBS)

encode: encode.o libcodecA.so libcodecB.so
	$(CC) $(CFLAGS) encode.o -L. -lcodecA -lcodecB -o encode $(LIBS)

decode: decode.o libcodecA.so libcodecB.so
	$(CC) $(CFLAGS) decode.o -L. -lcodecA -lcodecB -o decode $(LIBS)

libcodecA.so: codecA.o
	$(CC) $(CFLAGS) -shared -o libcodecA.so codecA.o

libcodecB.so: codecB.o
	$(CC) $(CFLAGS) -shared -o libcodecB.so codecB.o

cmp.o: cmp.c
	$(CC) $(CFLAGS) -c cmp.c

copy.o: copy.c
	$(CC) $(CFLAGS) -c copy.c

encode.o: encode.c
	$(CC) $(CFLAGS) -c encode.c

decode.o: decode.c
	$(CC) $(CFLAGS) -c decode.c

codecA.o: codecA.c
	$(CC) $(CFLAGS) -c -fPIC codecA.c

codecB.o: codecB.c
	$(CC) $(CFLAGS) -c -fPIC codecB.c

clean:
	rm -f *.o *.so cmp copy encode decode

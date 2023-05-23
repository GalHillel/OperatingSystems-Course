CC = gcc
CFLAGS = -Wall -Wextra -pedantic -pthread

all: react_server

react_server: react_server.o libst_reactor.so
	$(CC) $(CFLAGS) -o react_server react_server.o -L. -Wl,-rpath=. -lst_reactor

react_server.o: react_server.c reactor.h
	$(CC) $(CFLAGS) -c react_server.c

libst_reactor.so: reactor.o
	$(CC) $(CFLAGS) -shared -o libst_reactor.so reactor.o

reactor.o: reactor.c reactor.h
	$(CC) $(CFLAGS) -c reactor.c

clean:
	rm -f react_server libst_reactor.so *.o

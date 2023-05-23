#ifndef REACTOR_H
#define REACTOR_H

#include <sys/types.h>

typedef struct Reactor Reactor;
typedef void (*handler_t)(int fd);

Reactor *createReactor();
void stopReactor(Reactor *reactor);
void startReactor(Reactor *reactor);
void addFd(Reactor *reactor, int fd, handler_t handler);
void waitForReactor(Reactor *reactor);

#endif

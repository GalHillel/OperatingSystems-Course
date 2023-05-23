#include "reactor.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <string.h>

#define MAX_FDS 1024

typedef struct Reactor
{
    fd_set read_fds;
    int max_fd;
    handler_t handlers[MAX_FDS];
    int running;
} Reactor;

Reactor *createReactor()
{
    printf("Creating Reactor...\n");
    Reactor *reactor = (Reactor *)malloc(sizeof(Reactor));
    if (reactor == NULL)
    {
        perror("Failed to allocate memory for reactor");
        exit(1);
    }
    FD_ZERO(&(reactor->read_fds));
    reactor->max_fd = -1;
    reactor->running = 0;
    memset(reactor->handlers, 0, MAX_FDS * sizeof(handler_t));
    printf("Reactor Created Successfully\n");
    return reactor;
}

void stopReactor(Reactor *reactor)
{
    reactor->running = 0;
}

void startReactor(Reactor *reactor)
{
    reactor->running = 1;
    while (reactor->running)
    {
        fd_set temp_fds = reactor->read_fds;
        if (select(reactor->max_fd + 1, &temp_fds, NULL, NULL, NULL) == -1)
        {
            perror("Error in select");
            exit(1);
        }
        for (int fd = 0; fd <= reactor->max_fd; fd++)
        {
            if (FD_ISSET(fd, &temp_fds))
            {
                handler_t handler = reactor->handlers[fd];
                if (handler != NULL)
                {
                    handler(fd);
                }
            }
        }
    }
}

void addFd(Reactor *reactor, int fd, handler_t handler)
{
    FD_SET(fd, &(reactor->read_fds));
    if (fd > reactor->max_fd)
    {
        reactor->max_fd = fd;
    }
    reactor->handlers[fd] = handler;
}

void waitForReactor(Reactor *reactor)
{
    while (reactor->running)
    {
        usleep(1000);
    }
    free(reactor);
}

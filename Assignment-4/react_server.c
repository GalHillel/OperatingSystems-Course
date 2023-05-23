#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdint.h>
#include <netinet/in.h>
#include "reactor.h"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define PORT 9034

int client_sockets[MAX_CLIENTS];
pthread_t client_threads[MAX_CLIENTS];
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
Reactor *reactor;

void handleClient(int client_fd)
{
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    ssize_t recv_size = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if (recv_size > 0)
    {
        printf("Received message from client %d: %s", client_fd, buffer);
    }
    else if (recv_size == 0)
    {
        printf("Client %d disconnected\n", client_fd);
        close(client_fd);

        pthread_mutex_lock(&client_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_sockets[i] == client_fd)
            {
                client_sockets[i] = 0;
                break;
            }
        }
        pthread_mutex_unlock(&client_mutex);
    }
    else
    {
        perror("Error receiving message from client");
    }
}

void acceptClient(int server_fd)
{
    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);
    if (client_fd < 0)
    {
        perror("Error accepting client connection");
        return;
    }

    printf("Client %d connected succsesfully\n", client_fd);

    pthread_mutex_lock(&client_mutex);
    int slot = -1;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (client_sockets[i] == 0)
        {
            client_sockets[i] = client_fd;
            slot = i;
            break;
        }
    }
    pthread_mutex_unlock(&client_mutex);

    if (slot >= 0)
    {
        handler_t handler = handleClient;
        addFd(reactor, client_fd, handler);
    }
    else
    {
        printf("Maximum number of clients reached. Connection closed: %d\n", client_fd);
        close(client_fd);
    }
}

int main()
{
    int server_fd;
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("Error creating socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding socket");
        exit(1);
    }

    if (listen(server_fd, 10) < 0)
    {
        perror("Error listening on socket");
        exit(1);
    }

    reactor = createReactor();
    handler_t handler = acceptClient;
    addFd(reactor, server_fd, handler);

    printf("Starting Server...\nListening on port %d...\n", PORT);

    startReactor(reactor);

    waitForReactor(reactor);

    close(server_fd);

    return 0;
}

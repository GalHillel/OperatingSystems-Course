#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void run_client(const char *ip, int port);
void run_server(int port);

void handle_error(const char *error_message)
{
    perror(error_message);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage:\n");
        printf("Client side: stnc -c IP PORT\n");
        printf("Server side: stnc -s PORT\n");
        return EXIT_SUCCESS;
    }

    int is_client = 0;
    int is_server = 0;
    const char *ip = NULL;
    int port = 0;

    if (strcmp(argv[1], "-c") == 0)
    {
        is_client = 1;
        ip = argv[2];
        port = atoi(argv[3]);
    }
    else if (strcmp(argv[1], "-s") == 0)
    {
        is_server = 1;
        port = atoi(argv[2]);
    }
    else
    {
        printf("Invalid option.\n");
        return EXIT_FAILURE;
    }

    if (is_client)
    {
        run_client(ip, port);
    }
    else if (is_server)
    {
        run_server(port);
    }

    return EXIT_SUCCESS;
}

void run_client(const char *ip, int port)
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        handle_error("Socket creation failed");
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        handle_error("Invalid address or address not supported");
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        handle_error("Connection failed");
    }

    // Start chat loop
    while (1)
    {
        printf("Client: ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // Send message
        send(sock, buffer, strlen(buffer), 0);

        // Receive response
        memset(buffer, 0, sizeof(buffer));
        if (recv(sock, buffer, BUFFER_SIZE, 0) <= 0)
        {
            break;
        }

        printf("Server: %s", buffer);
    }

    close(sock);
}

void run_server(int port)
{
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        handle_error("Socket creation failed");
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        handle_error("Binding failed");
    }

    if (listen(server_sock, 1) < 0)
    {
        handle_error("Listening failed");
    }

    printf("Server listening on port %d\n", port);

    client_len = sizeof(client_addr);
    if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len)) < 0)
    {
        handle_error("Accepting connection failed");
    }

    // Start chat loop
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));

        // Receive message
        if (recv(client_sock, buffer, BUFFER_SIZE, 0) <= 0)
        {
            break;
        }

        printf("Client: %s", buffer);

        // Send response
        printf("Server: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        send(client_sock, buffer, strlen(buffer), 0);
    }

    close(client_sock);
    close(server_sock);
}

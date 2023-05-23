#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/un.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define DATA_SIZE (100 * 1024 * 1024) // 100 MB

int accept_connection(int server_sock);
void handle_client_request(int client_sock);
void generate_data(char *buffer);
int tcp_ipv4_client(const char *ip, int port);
int tcp_ipv4_server(int port);
int tcp_ipv6_client(const char *ip, int port);
int tcp_ipv6_server(int port);
int uds_stream_client(const char *socket_path);
int uds_stream_server(const char *socket_path);
int uds_dgram_client(const char *socket_path);
int uds_dgram_server(const char *socket_path);
int mmap_client(const char *filename);
int mmap_server(const char *filename);
int pipe_client(const char *filename);
int pipe_server(const char *filename);
void run_server(int port, int performance_test, int quiet_mode);
void run_client(const char *ip, int port, const char *param);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s -s port [-p] [-q] | -c ip port -p type param\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int option;
    int server = 0;
    int client = 0;
    int performance_test = 0;
    int quiet_mode = 0;
    char *ip = NULL;
    int port = 0;
    char *param = NULL;

    while ((option = getopt(argc, argv, "sc:p:q")) != -1)
    {
        switch (option)
        {
        case 's':
            server = 1;
            port = atoi(argv[2]);
            break;
        case 'c':
            client = 1;
            ip = optarg;
            break;
        case 'p':
            param = optarg;
            break;
        case 'q':
            quiet_mode = 1;
            break;
        default:
            fprintf(stderr, "Invalid option\n");
            exit(EXIT_FAILURE);
        }
    }

    if (server)
    {
        run_server(port, performance_test, quiet_mode);
    }
    else if (client)
    {
        run_client(ip, port, param);
    }
    else
    {
        fprintf(stderr, "Invalid command\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

int accept_connection(int server_sock)
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Accept a client connection
    int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_sock < 0)
    {
        perror("Error accepting client connection");
        exit(EXIT_FAILURE);
    }

    // Print client connection information
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    printf("Accepted connection from client: %s:%d\n", client_ip, ntohs(client_addr.sin_port));

    return client_sock;
}

void handle_client_request(int client_sock)
{
    char buffer[BUFFER_SIZE];
    ssize_t num_bytes;

    // Read data from the client
    num_bytes = recv(client_sock, buffer, BUFFER_SIZE, 0);
    if (num_bytes < 0)
    {
        perror("Error reading from client socket");
        exit(EXIT_FAILURE);
    }

    // Perform necessary operations based on the received data
    // In this example, we simply print the received message
    printf("Received message from client: %.*s", (int)num_bytes, buffer);

    // Send response back to the client
    const char *response = "Server response";
    num_bytes = send(client_sock, response, strlen(response), 0);
    if (num_bytes < 0)
    {
        perror("Error sending response to client");
        exit(EXIT_FAILURE);
    }
}

void generate_data(char *buffer)
{
    for (int i = 0; i < DATA_SIZE; ++i)
    {
        buffer[i] = (char)(rand() % 256);
    }
}

int tcp_ipv4_client(const char *ip, int port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &(server_addr.sin_addr)) <= 0)
    {
        perror("Invalid IP address");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    return sock;
}

int tcp_ipv4_server(int port)
{
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 5) < 0)
    {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for client connection...\n");

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
    if (client_sock < 0)
    {
        perror("Accepting failed");
        exit(EXIT_FAILURE);
    }

    printf("Client connected\n");

    close(server_sock);
    return client_sock;
}

int tcp_ipv6_client(const char *ip, int port)
{
    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(port);

    if (inet_pton(AF_INET6, ip, &(server_addr.sin6_addr)) <= 0)
    {
        perror("Invalid IP address");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    return sock;
}

int tcp_ipv6_server(int port)
{
    int server_sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 5) < 0)
    {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for client connection...\n");

    struct sockaddr_in6 client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
    if (client_sock < 0)
    {
        perror("Accepting failed");
        exit(EXIT_FAILURE);
    }

    printf("Client connected\n");

    close(server_sock);
    return client_sock;
}

int uds_stream_client(const char *socket_path)
{
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    return sock;
}

int uds_stream_server(const char *socket_path)
{
    int server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);

    unlink(socket_path); // Remove the existing socket file if it exists

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 5) < 0)
    {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for client connection...\n");

    struct sockaddr_un client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
    if (client_sock < 0)
    {
        perror("Accepting failed");
        exit(EXIT_FAILURE);
    }

    printf("Client connected\n");

    close(server_sock);
    return client_sock;
}

int uds_dgram_client(const char *socket_path)
{
    int sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    return sock;
}

int uds_dgram_server(const char *socket_path)
{
    int server_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (server_sock < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);

    unlink(socket_path); // Remove the existing socket file if it exists

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for data...\n");

    char buffer[BUFFER_SIZE];
    struct sockaddr_un client_addr;
    socklen_t client_len = sizeof(client_addr);
    ssize_t recv_len = recvfrom(server_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
    if (recv_len < 0)
    {
        perror("Receiving failed");
        exit(EXIT_FAILURE);
    }

    printf("Data received from client\n");

    close(server_sock);
    return 0;
}

int mmap_client(const char *filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        perror("File opening failed");
        exit(EXIT_FAILURE);
    }

    struct stat file_stat;
    if (fstat(fd, &file_stat) < 0)
    {
        perror("File stat failed");
        exit(EXIT_FAILURE);
    }

    char *mapped_data = mmap(NULL, file_stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (mapped_data == MAP_FAILED)
    {
        perror("Memory mapping failed");
        exit(EXIT_FAILURE);
    }

    close(fd);

    return 0;
}

int mmap_server(const char *filename)
{
    int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        perror("File opening failed");
        exit(EXIT_FAILURE);
    }

    if (lseek(fd, DATA_SIZE - 1, SEEK_SET) < 0)
    {
        perror("File seek failed");
        exit(EXIT_FAILURE);
    }

    if (write(fd, "", 1) < 0)
    {
        perror("File write failed");
        exit(EXIT_FAILURE);
    }

    char *mapped_data = mmap(NULL, DATA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_data == MAP_FAILED)
    {
        perror("Memory mapping failed");
        exit(EXIT_FAILURE);
    }

    close(fd);

    return 0;
}

int pipe_client(const char *filename)
{
    int fd = open(filename, O_WRONLY);
    if (fd < 0)
    {
        perror("Pipe opening failed");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_written = write(fd, buffer, BUFFER_SIZE);
    if (bytes_written < 0)
    {
        perror("Write failed");
        exit(EXIT_FAILURE);
    }

    close(fd);

    return 0;
}

int pipe_server(const char *filename)
{
    if (mkfifo(filename, 0666) < 0)
    {
        perror("FIFO creation failed");
        exit(EXIT_FAILURE);
    }

    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        perror("Pipe opening failed");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE);
    if (bytes_read < 0)
    {
        perror("Read failed");
        exit(EXIT_FAILURE);
    }

    close(fd);

    return 0;
}

void run_server(int port, int performance_test, int quiet_mode)
{
    if (!performance_test)
    {
        printf("Starting server...\n");
    }

    int server_sock;

    if (port < 0 || port > 65535)
    {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }

    if (performance_test)
    {
        // Perform the performance test
        double start_time, end_time;
        start_time = (double)clock() / CLOCKS_PER_SEC;

        switch (port)
        {
        case 1:
            // TCP IPv4
            server_sock = tcp_ipv4_server(htons(port));
            break;
        case 2:
            // TCP IPv6
            server_sock = tcp_ipv6_server(htons(port));
            break;
        case 3:
            // UDS DGRAM
            server_sock = uds_dgram_server("socket_path");
            break;
        case 4:
            // UDS STREAM
            server_sock = uds_stream_server("socket_path");
            break;
        case 5:
            // MMAP
            server_sock = mmap_server("file_path");
            break;
        case 6:
            // PIPE
            server_sock = pipe_server("pipe_path");
            break;
        default:
            fprintf(stderr, "Invalid port number for performance test\n");
            exit(EXIT_FAILURE);
        }

        end_time = (double)clock() / CLOCKS_PER_SEC;
        double elapsed_time = (end_time - start_time) * 1000; // Convert to milliseconds

        if (!quiet_mode)
        {
            switch (port)
            {
            case 1:
                printf("ipv4_tcp,%.0f\n", elapsed_time);
                break;
            case 2:
                printf("ipv6_tcp,%.0f\n", elapsed_time);
                break;
            case 3:
                printf("uds_dgram,%.0f\n", elapsed_time);
                break;
            case 4:
                printf("uds_stream,%.0f\n", elapsed_time);
                break;
            case 5:
                printf("mmap,%.0f\n", elapsed_time);
                break;
            case 6:
                printf("pipe,%.0f\n", elapsed_time);
                break;
            }
        }
    }
    else
    {
        // Run the server normally

        switch (port)
        {
        case 1:
            // TCP IPv4
            server_sock = tcp_ipv4_server(htons(port));
            break;
        case 2:
            // TCP IPv6
            server_sock = tcp_ipv6_server(htons(port));
            break;
        case 3:
            // UDS DGRAM
            server_sock = uds_dgram_server("socket_path");
            break;
        case 4:
            // UDS STREAM
            server_sock = uds_stream_server("socket_path");
            break;
        case 5:
            // MMAP
            server_sock = mmap_server("file_path");
            break;
        case 6:
            // PIPE
            server_sock = pipe_server("pipe_path");
            break;
        default:
            fprintf(stderr, "Invalid port number\n");
            exit(EXIT_FAILURE);
        }

        // Continue running the server until interrupted
        while (1)
        {
            // Accept a client connection
            int client_sock = accept_connection(server_sock);

            if (client_sock == -1)
            {
                fprintf(stderr, "Error accepting connection\n");
                continue;
            }

            // Handle client request
            handle_client_request(client_sock);

            // Close the client socket
            close(client_sock);
        }
    }

    close(server_sock);
}

void run_client(const char *ip, int port, const char *param)
{
    printf("Starting client...\n");

    int client_sock;

    if (port < 0 || port > 65535)
    {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }

    if (param != NULL)
    {
        // Perform the performance test
        double start_time, end_time;
        start_time = (double)clock() / CLOCKS_PER_SEC;

        if (strcmp(param, "ipv4_tcp") == 0)
        {
            // TCP IPv4
            client_sock = tcp_ipv4_client(ip, port);
        }
        else if (strcmp(param, "ipv6_tcp") == 0)
        {
            // TCP IPv6
            // client_sock = tcp_ipv6_client(ip, port);
        }
        else if (strcmp(param, "uds_dgram") == 0)
        {
            // UDS DGRAM
            client_sock = uds_dgram_client("socket_path");
        }
        else if (strcmp(param, "uds_stream") == 0)
        {
            // UDS STREAM
            client_sock = uds_stream_client("socket_path");
        }
        else if (strcmp(param, "mmap") == 0)
        {
            // MMAP
            client_sock = mmap_client("file_path");
        }
        else if (strcmp(param, "pipe") == 0)
        {
            // PIPE
            client_sock = pipe_client("pipe_path");
        }
        else
        {
            fprintf(stderr, "Invalid parameter\n");
            exit(EXIT_FAILURE);
        }

        end_time = (double)clock() / CLOCKS_PER_SEC;
        double elapsed_time = (end_time - start_time) * 1000; // Convert to milliseconds

        printf("Time: %.0f ms\n", elapsed_time);
    }
    else
    {
        fprintf(stderr, "Parameter missing\n");
        exit(EXIT_FAILURE);
    }

    close(client_sock);
}

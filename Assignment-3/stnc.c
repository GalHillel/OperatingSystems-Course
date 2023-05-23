#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <poll.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 32768
#define BUFFER_SIZE_MESSAGE 1024

void generateFile(char *filename, long sizeInBytes, int quiet);
uint32_t calculateChecksum(char *filename, int quiet);
int deleteFile(char *filename, int quiet);
void printTimeDifference(struct timeval *start, struct timeval *end);
void sendFile(char *ip, char *port, char *filename, int domain, int type, int protocol, int quiet);
int receiveFile(char *port, int domain, int type, int protocol, int fileSize, int quiet);
void copyFileMmap(char *filenameFrom, char *filenameTo);
void copyFilePipe(char *filenameFrom, char *filenameTo);
int getMinimum(int a, int b);
int getFileSize(char *filename);
void copyFileToSharedMemoryMmap(char *filenameFrom, char *sharedFilename, int quiet);
void copyFileFromSharedMemoryMmap(char *filenameTo, char *sharedFilename, int fileSize, int quiet);
void receiveFileFifo(char *filenameTo, char *fifoName, int quiet);
void sendFileFifo(char *filenameFrom, char *fifoName, int quiet);
void run_client(char *ip, char *port);
void run_server(char *port);

int client = 0;
int server = 0;
int test = 0;
int ipv4 = 0;
int ipv6 = 0;
int tcp = 0;
int udp = 0;
int uds = 0;
int dgram = 0;
int stream = 0;
int isMmap = 0;
int isPipe = 0;
int deletFile = 0;
int quiet = 0;
char *filename = NULL;
char *ip = NULL;
char *port = NULL;
char *type = NULL;
char *param = NULL;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s -s port [-p] [-q] | -c ip port -p type param\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-c") == 0)
        {
            client = 1;
            ip = argv[i + 1];
            port = argv[i + 2];
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            server = 1;
            port = argv[i + 1];
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            test = 1;
            type = argv[i + 1];
            param = argv[i + 2];
        }
        else if (strcmp(argv[i], "-q") == 0)
        {
            quiet = 1;
        }
    }

    if (test && client)
    {
        if (!type || !param)
        {
            printf("No type or param after -p\n");
            return 1;
        }
        if (strcmp(type, "ipv4") == 0)
        {
            ipv4 = 1;
        }
        else if (strcmp(type, "ipv6") == 0)
        {
            ipv6 = 1;
        }
        else if (strcmp(type, "uds") == 0)
        {
            uds = 1;
        }
        else if (strcmp(type, "mmap") == 0)
        {
            isMmap = 1;
        }
        else if (strcmp(type, "pipe") == 0)
        {
            isPipe = 1;
        }

        if (strcmp(param, "tcp") == 0)
        {
            tcp = 1;
        }
        else if (strcmp(param, "udp") == 0)
        {
            udp = 1;
        }
        else if (strcmp(param, "dgram") == 0)
        {
            dgram = 1;
        }
        else if (strcmp(param, "stream") == 0)
        {
            stream = 1;
        }
        else
        {
            filename = param;
        }
    }

    if (client)
    {
        run_client(ip, port);
    }
    else if (server)
    {
        run_server(port);
    }
    else
    {
        fprintf(stderr, "Usage: %s -s port [-p] [-q] | -c ip port -p type param\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

void run_client(char *ip, char *port)
{
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        printf("ERROR opening socket\n");
        exit(1);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));
    int rval = inet_pton(AF_INET, ip, &serv_addr.sin_addr);
    if (rval <= 0)
    {
        printf("ERROR inet_pton() failed\n");
        exit(1);
    }
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("ERROR connecting to %s:%s\n", ip, port);
        exit(1);
    }

    if (!quiet)
        printf("Connected to %s:%s\n", ip, port);

    struct pollfd fds[2] = {
        {.fd = STDIN_FILENO, .events = POLLIN},
        {.fd = sockfd, .events = POLLIN}};

    char messageBuffer[BUFFER_SIZE_MESSAGE];
    int timeout = -1;

    while (1)
    {
        if (test)
        {
            generateFile("test.txt", 100 * 1024 * 1024, quiet);
            char new_port[10];
            sprintf(new_port, "%d", atoi(port) + 1);

            int filesize = getFileSize("test.txt");
            char filesize_str[20];
            sprintf(filesize_str, "%d", filesize);
            int bytesSent = send(sockfd, filesize_str, strlen(filesize_str), 0);
            if (bytesSent < 0)
            {
                printf("ERROR send() failed\n");
                exit(1);
            }
            sleep(0.1);

            uint32_t checksum = calculateChecksum("test.txt", quiet);
            char checksum_str[20];
            sprintf(checksum_str, "%d", checksum);
            bytesSent = send(sockfd, checksum_str, strlen(checksum_str), 0);
            if (bytesSent < 0)
            {
                printf("ERROR send() failed\n");
                exit(1);
            }
            sleep(0.1);

            struct timeval start;
            gettimeofday(&start, NULL);

            char start_time_str[20];
            sprintf(start_time_str, "%ld.%06ld", start.tv_sec, start.tv_usec);
            bytesSent = send(sockfd, start_time_str, strlen(start_time_str), 0);
            if (bytesSent < 0)
            {
                printf("ERROR send() failed\n");
                exit(1);
            }
            sleep(0.1);

            if (tcp && ipv4)
            {
                if (!quiet)
                    printf("TCP IPv4\n");
                bytesSent = send(sockfd, "ipv4 tcp", 8, 0);
            }
            else if (udp && ipv4)
            {
                if (!quiet)
                    printf("UDP IPv4\n");
                bytesSent = send(sockfd, "ipv4 udp", 8, 0);
            }
            else if (tcp && ipv6)
            {
                if (!quiet)
                    printf("TCP IPv6\n");
                bytesSent = send(sockfd, "ipv6 tcp", 8, 0);
            }
            else if (udp && ipv6)
            {
                if (!quiet)
                    printf("UDP IPv6\n");
                bytesSent = send(sockfd, "ipv6 udp", 8, 0);
            }
            else if (uds && dgram)
            {
                if (!quiet)
                    printf("UDS DGRAM\n");
                bytesSent = send(sockfd, "uds dgram", 9, 0);
            }
            else if (uds && stream)
            {
                if (!quiet)
                    printf("UDS STREAM\n");
                bytesSent = send(sockfd, "uds stream", 10, 0);
            }
            else if (isMmap)
            {
                if (!quiet)
                    printf("MMAP\n");
                bytesSent = send(sockfd, "mmap", 5, 0);
            }
            else if (isPipe)
            {
                if (!quiet)
                    printf("PIPE\n");
                bytesSent = send(sockfd, "pipe", 5, 0);
            }
            if (bytesSent < 0)
            {
                printf("ERROR send() failed\n");
                exit(1);
            }

            sleep(0.1);

            if (tcp && ipv4)
            {
                sendFile(ip, new_port, "test.txt", AF_INET, SOCK_STREAM, IPPROTO_TCP, quiet);
            }
            else if (udp && ipv4)
            {
                sendFile(ip, new_port, "test.txt", AF_INET, SOCK_DGRAM, 0, quiet);
            }
            else if (tcp && ipv6)
            {
                sendFile(ip, new_port, "test.txt", AF_INET6, SOCK_STREAM, IPPROTO_TCP, quiet);
            }
            else if (udp && ipv6)
            {
                sendFile(ip, new_port, "test.txt", AF_INET6, SOCK_DGRAM, 0, quiet);
            }
            else if (uds && dgram)
            {
                sleep(0.1);
                sendFile(0, new_port, "test.txt", AF_UNIX, SOCK_DGRAM, 0, quiet);
            }
            else if (uds && stream)
            {
                sleep(0.1);
                sendFile(0, new_port, "test.txt", AF_UNIX, SOCK_STREAM, 0, quiet);
            }
            else if (isMmap)
            {

                copyFileToSharedMemoryMmap("test.txt", filename, quiet);
                bytesSent = send(sockfd, filename, strlen(filename), 0);
                if (bytesSent < 0)
                {
                    printf("ERROR send() failed\n");
                    exit(1);
                }
            }
            else if (isPipe)
            {
                bytesSent = send(sockfd, filename, strlen(filename), 0);
                if (bytesSent < 0)
                {
                    printf("ERROR send() failed\n");
                    exit(1);
                }
                sendFileFifo("test.txt", filename, quiet);
            }
            deleteFile("test.txt", quiet);
            exit(0);
        }
        int pollResult = poll(fds, 2, timeout);
        if (pollResult < 0)
        {
            printf("ERROR poll() failed\n");
            exit(1);
        }

        if (fds[0].revents & POLLIN)
        {
            int bytesRead = read(STDIN_FILENO, messageBuffer, BUFFER_SIZE_MESSAGE);
            if (bytesRead < 0)
            {
                printf("ERROR read() failed\n");
                exit(1);
            }
            messageBuffer[bytesRead] = '\0';

            int bytesSent = send(sockfd, messageBuffer, bytesRead, 0);
            if (bytesSent < 0)
            {
                printf("ERROR send() failed\n");
                exit(1);
            }
            bzero(messageBuffer, BUFFER_SIZE_MESSAGE);
        }
        if (fds[1].revents & POLLIN)
        {
            int bytesRecv = recv(sockfd, messageBuffer, BUFFER_SIZE_MESSAGE - 1, 0);
            if (bytesRecv < 0)
            {
                printf("ERROR recv() failed\n");
                exit(1);
            }
            if (bytesRecv == 0)
            {
                printf("Server disconnected\n");
                exit(1);
            }
            messageBuffer[bytesRecv] = '\0';
            printf("Server: %s", messageBuffer);
            bzero(messageBuffer, BUFFER_SIZE_MESSAGE);
        }
    }
    close(sockfd);
}

void run_server(char *port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        printf("ERROR opening socket\n");
        exit(1);
    }

    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(atoi(port))};
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("ERROR on binding\n");
        exit(1);
    }
    while (1)
    {

        if (listen(sockfd, 1) < 0)
        {
            printf("ERROR on listen\n");
            exit(1);
        }

        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int clientSock = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (clientSock < 0)
        {
            printf("ERROR on accept\n");
            exit(1);
        }

        if (!quiet)
            printf("Client connected\n");

        struct pollfd fds[2] = {
            {.fd = STDIN_FILENO, .events = POLLIN},
            {.fd = clientSock, .events = POLLIN}};

        char messageBuffer[BUFFER_SIZE_MESSAGE];

        while (1)
        {
            int pllResult = poll(fds, 2, -1);
            if (pllResult < 0)
            {
                printf("ERROR poll() failed\n");
                exit(1);
            }
            if (fds[0].revents & POLLIN)
            {
                int bytesRead = read(STDIN_FILENO, messageBuffer, BUFFER_SIZE_MESSAGE);
                if (bytesRead < 0)
                {
                    printf("ERROR read() failed\n");
                    exit(1);
                }
                messageBuffer[bytesRead] = '\0';
                if (send(clientSock, messageBuffer, bytesRead, 0) < 0)
                {
                    printf("ERROR send() failed\n");
                    exit(1);
                }
                bzero(messageBuffer, BUFFER_SIZE_MESSAGE);
            }
            if (fds[1].revents & POLLIN)
            {
                int bytesRecv = recv(clientSock, messageBuffer, BUFFER_SIZE_MESSAGE - 1, 0);
                if (bytesRecv < 0)
                {
                    printf("ERROR recv() failed\n");
                    exit(1);
                }
                if (bytesRecv == 0)
                {
                    if (!quiet)
                        printf("Client disconnected\n");
                    break;
                }
                messageBuffer[bytesRecv] = '\0';
                if (!quiet)
                    printf("Client: %s", messageBuffer);
                if (test)
                {
                    int fileSize = 0;
                    int recievedSize = 0;
                    uint32_t checksum = 0;
                    struct timeval start, end;
                    if (!quiet)
                        fileSize = atoi(messageBuffer);
                    if (!quiet)
                        printf("File size will be: %d\n", fileSize);
                    bzero(messageBuffer, BUFFER_SIZE_MESSAGE);

                    bytesRecv = recv(clientSock, messageBuffer, 20, 0);
                    if (bytesRecv <= 0)
                    {
                        printf("ERROR recv() failed\n");
                        exit(1);
                    }
                    messageBuffer[bytesRecv] = '\0';
                    sscanf(messageBuffer, "%u", &checksum);

                    if (!quiet)
                        printf("Checksum will be: 0x%08x\n", checksum);
                    bzero(messageBuffer, BUFFER_SIZE_MESSAGE);

                    bytesRecv = recv(clientSock, messageBuffer, 20, 0);
                    if (bytesRecv <= 0)
                    {
                        printf("ERROR recv() failed\n");
                        exit(1);
                    }
                    messageBuffer[bytesRecv] = '\0';
                    sscanf(messageBuffer, "%ld.%06ld", &start.tv_sec, &start.tv_usec);
                    if (!quiet)
                        printf("Start time: %ld.%06ld\n", start.tv_sec, start.tv_usec);
                    bzero(messageBuffer, BUFFER_SIZE_MESSAGE);

                    bytesRecv = recv(clientSock, messageBuffer, 20, 0);
                    if (bytesRecv <= 0)
                    {
                        printf("ERROR recv() failed\n");
                        exit(1);
                    }
                    messageBuffer[bytesRecv] = '\0';
                    if (!quiet)
                        printf("Test: %s\n", messageBuffer);

                    char *new_port[10];
                    sprintf(new_port, "%d", atoi(port) + 1);

                    if (!strcmp(messageBuffer, "ipv4 tcp"))
                    {
                        recievedSize = receiveFile(new_port, AF_INET, SOCK_STREAM, IPPROTO_TCP, fileSize, quiet);
                    }
                    else if (!strcmp(messageBuffer, "ipv4 udp"))
                    {
                        recievedSize = receiveFile(new_port, AF_INET, SOCK_DGRAM, 0, fileSize, quiet);
                    }
                    else if (!strcmp(messageBuffer, "ipv6 tcp"))
                    {
                        recievedSize = receiveFile(new_port, AF_INET6, SOCK_STREAM, IPPROTO_TCP, fileSize, quiet);
                    }
                    else if (!strcmp(messageBuffer, "ipv6 udp"))
                    {
                        recievedSize = receiveFile(new_port, AF_INET6, SOCK_DGRAM, 0, fileSize, quiet);
                    }
                    else if (!strcmp(messageBuffer, "uds dgram"))
                    {
                        recievedSize = receiveFile(new_port, AF_UNIX, SOCK_DGRAM, 0, fileSize, quiet);
                    }
                    else if (!strcmp(messageBuffer, "uds stream"))
                    {
                        recievedSize = receiveFile(new_port, AF_UNIX, SOCK_STREAM, 0, fileSize, quiet);
                    }
                    else if (!strcmp(messageBuffer, "mmap"))
                    {
                        bytesRecv = recv(clientSock, messageBuffer, BUFFER_SIZE_MESSAGE - 1, 0);
                        if (bytesRecv < 0)
                        {
                            printf("ERROR recv() failed\n");
                            exit(1);
                        }
                        if (!quiet)
                            printf("Shared file name: %s\n", messageBuffer);
                        sleep(0.1);
                        copyFileFromSharedMemoryMmap("recived.txt", messageBuffer, fileSize, quiet);
                        recievedSize = getFileSize("recived.txt");
                    }
                    else if (!strcmp(messageBuffer, "pipe"))
                    {
                        bytesRecv = recv(clientSock, messageBuffer, BUFFER_SIZE_MESSAGE - 1, 0);
                        if (bytesRecv < 0)
                        {
                            printf("ERROR recv() failed\n");
                            exit(1);
                        }
                        if (!quiet)
                            printf("Shared fifo name: %s\n", messageBuffer);
                        sleep(0.1);
                        receiveFileFifo("recived.txt", messageBuffer, quiet);
                        recievedSize = getFileSize("recived.txt");
                    }

                    gettimeofday(&end, NULL);

                    u_int32_t recieved_file_checksum = calculateChecksum("recived.txt", quiet);
                    if (recieved_file_checksum == checksum && !quiet)
                    {
                        printf("Checksums are equal\n");
                    }
                    else if (!quiet)
                    {
                        printf("Checksums are not equal\n");
                        if (recievedSize != fileSize)
                        {
                            printf("File sizes are not equal packets were lost\n");
                        }
                    }

                    if (!quiet)
                    {
                        printf("End time: %ld.%06ld\n", end.tv_sec, end.tv_usec);
                        printf("Time took: ");
                    }

                    printTimeDifference(&start, &end);

                    deleteFile("recived.txt", quiet);
                }
                bzero(messageBuffer, BUFFER_SIZE_MESSAGE);
            }
        }
        close(clientSock);
    }
    close(sockfd);
}

void generateFile(char *filename, long sizeInBytes, int quiet)
{
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("Error opening file '%s'\n", filename);
        return;
    }

    const int chunkSize = 1024 * 1024;
    char buffer[chunkSize];
    int bytesWritten = 0;

    while (bytesWritten < sizeInBytes)
    {
        int bytesToWrite = chunkSize;
        if (bytesWritten + bytesToWrite > sizeInBytes)
        {
            bytesToWrite = sizeInBytes - bytesWritten;
        }
        fwrite(buffer, bytesToWrite, 1, fp);
        bytesWritten += bytesToWrite;
    }

    fclose(fp);
    if (!quiet)
        printf("Generated file '%s' of size %ld bytes\n", filename, sizeInBytes);
}

uint32_t calculateChecksum(char *filename, int quiet)
{
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        printf("Error opening file '%s'\n", filename);
        return -1;
    }

    const int chunkSize = 1024 * 1024;
    char buffer[chunkSize];
    uint32_t checksum = 0;

    while (!feof(fp))
    {
        size_t bytesRead = fread(buffer, 1, chunkSize, fp);
        for (size_t i = 0; i < bytesRead; i++)
        {
            checksum += (uint32_t)buffer[i];
        }
    }

    fclose(fp);
    if (!quiet)
        printf("Generated checksum for file '%s': 0x%08x\n", filename, checksum);
    return checksum;
}

int deleteFile(char *filename, int quiet)
{
    int status = remove(filename);
    if (status != 0)
    {
        printf("Error deleting file '%s'\n", filename);
        return -1;
    }
    if (!quiet)
        printf("File '%s' deleted successfully\n", filename);
    return 0;
}

void printTimeDifference(struct timeval *start, struct timeval *end)
{
    int seconds = end->tv_sec - start->tv_sec;
    int microseconds = end->tv_usec - start->tv_usec;
    if (microseconds < 0)
    {
        seconds--;
        microseconds += 1000000;
    }
    printf("Time difference: %d.%06d seconds\n", seconds, microseconds);
}

void sendFile(char *ip, char *port, char *filename, int domain, int type, int protocol, int quiet)
{
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        printf("Error opening file '%s'\n", filename);
        return;
    }

    int sockfd = socket(domain, type, protocol);
    if (sockfd < 0)
    {
        printf("Error creating socket\n");
        fclose(fp);
        return;
    }

    struct sockaddr_in serverAddr;
    bzero((char *)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = domain;
    serverAddr.sin_port = htons(atoi(port));
    if (inet_pton(domain, ip, &(serverAddr.sin_addr)) <= 0)
    {
        printf("Error setting server address\n");
        fclose(fp);
        close(sockfd);
        return;
    }

    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("Error connecting to the server\n");
        fclose(fp);
        close(sockfd);
        return;
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char buffer[BUFFER_SIZE];
    int bytesSent = 0;
    int totalBytesSent = 0;

    while (totalBytesSent < fileSize)
    {
        int bytesRead = fread(buffer, 1, BUFFER_SIZE, fp);
        if (bytesRead <= 0)
            break;

        int bytesRemaining = fileSize - totalBytesSent;
        int bytesToSend = getMinimum(bytesRead, bytesRemaining);
        int bytesSent = send(sockfd, buffer, bytesToSend, 0);
        if (bytesSent < 0)
        {
            printf("Error sending file '%s'\n", filename);
            fclose(fp);
            close(sockfd);
            return;
        }
        totalBytesSent += bytesSent;
    }

    fclose(fp);
    close(sockfd);

    if (!quiet)
        printf("File '%s' sent successfully\n", filename);
}

int receiveFile(char *port, int domain, int type, int protocol, int fileSize, int quiet)
{
    int sockfd = socket(domain, type, protocol);
    if (sockfd < 0)
    {
        printf("Error creating socket\n");
        return -1;
    }

    struct sockaddr_in serverAddr, clientAddr;
    bzero((char *)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = domain;
    serverAddr.sin_port = htons(atoi(port));
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("Error binding socket\n");
        close(sockfd);
        return -1;
    }

    listen(sockfd, 5);

    socklen_t clientLen = sizeof(clientAddr);
    int clientSocket = accept(sockfd, (struct sockaddr *)&clientAddr, &clientLen);
    if (clientSocket < 0)
    {
        printf("Error accepting connection\n");
        close(sockfd);
        return -1;
    }

    FILE *fp = fopen("received_file.bin", "wb");
    if (fp == NULL)
    {
        printf("Error creating file\n");
        close(clientSocket);
        close(sockfd);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    int bytesRead = 0;
    int totalBytesRead = 0;

    while (totalBytesRead < fileSize)
    {
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesRead <= 0)
            break;

        fwrite(buffer, bytesRead, 1, fp);
        totalBytesRead += bytesRead;
    }

    fclose(fp);
    close(clientSocket);
    close(sockfd);

    if (!quiet)
        printf("File received successfully\n");

    return 0;
}

void copyFileMmap(char *filenameFrom, char *filenameTo)
{
    int fdFrom = open(filenameFrom, O_RDONLY);
    if (fdFrom < 0)
    {
        printf("Error opening file '%s'\n", filenameFrom);
        return;
    }

    struct stat statBuf;
    if (fstat(fdFrom, &statBuf) < 0)
    {
        printf("Error getting file stats\n");
        close(fdFrom);
        return;
    }

    int fdTo = open(filenameTo, O_RDWR | O_CREAT | O_TRUNC, statBuf.st_mode);
    if (fdTo < 0)
    {
        printf("Error creating file '%s'\n", filenameTo);
        close(fdFrom);
        return;
    }

    off_t fileSize = statBuf.st_size;

    if (ftruncate(fdTo, fileSize) < 0)
    {
        printf("Error setting file size\n");
        close(fdFrom);
        close(fdTo);
        return;
    }

    void *src = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fdFrom, 0);
    if (src == MAP_FAILED)
    {
        printf("Error mapping source file\n");
        close(fdFrom);
        close(fdTo);
        return;
    }

    void *dst = mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fdTo, 0);
    if (dst == MAP_FAILED)
    {
        printf("Error mapping destination file\n");
        munmap(src, fileSize);
        close(fdFrom);
        close(fdTo);
        return;
    }

    memcpy(dst, src, fileSize);

    munmap(src, fileSize);
    munmap(dst, fileSize);
    close(fdFrom);
    close(fdTo);

    printf("File '%s' copied to '%s' using mmap\n", filenameFrom, filenameTo);
}

void copyFilePipe(char *filenameFrom, char *filenameTo)
{
    int fd[2];
    if (pipe(fd) < 0)
    {
        printf("Error creating pipe\n");
        return;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        printf("Error forking process\n");
        return;
    }

    if (pid == 0)
    {
        close(fd[0]);
        int fdFrom = open(filenameFrom, O_RDONLY);
        if (fdFrom < 0)
        {
            printf("Error opening file '%s'\n", filenameFrom);
            close(fd[1]);
            return;
        }
        struct stat statBuf;
        if (fstat(fdFrom, &statBuf) < 0)
        {
            printf("Error getting file stats\n");
            close(fd[1]);
            close(fdFrom);
            return;
        }
        off_t fileSize = statBuf.st_size;
        char buffer[BUFFER_SIZE];
        int bytesRead = 0;
        int totalBytesRead = 0;
        while (totalBytesRead < fileSize)
        {
            bytesRead = read(fdFrom, buffer, BUFFER_SIZE);
            if (bytesRead <= 0)
                break;
            write(fd[1], buffer, bytesRead);
            totalBytesRead += bytesRead;
        }
        close(fdFrom);
        close(fd[1]);
        printf("File '%s' copied to '%s' using pipe\n", filenameFrom, filenameTo);
    }
    else
    {
        close(fd[1]);
        int fdTo = open(filenameTo, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fdTo < 0)
        {
            printf("Error creating file '%s'\n", filenameTo);
            close(fd[0]);
            return;
        }
        char buffer[BUFFER_SIZE];
        int bytesRead = 0;
        while ((bytesRead = read(fd[0], buffer, BUFFER_SIZE)) > 0)
        {
            write(fdTo, buffer, bytesRead);
        }
        close(fdTo);
        close(fd[0]);
    }
}

int getMinimum(int a, int b)
{
    return (a < b) ? a : b;
}

int getFileSize(char *filename)
{
    struct stat statBuf;
    if (stat(filename, &statBuf) < 0)
    {
        printf("Error getting file stats\n");
        return -1;
    }
    return statBuf.st_size;
}

void copyFileToSharedMemoryMmap(char *filenameFrom, char *sharedFilename, int quiet)
{
    int fdFrom = open(filenameFrom, O_RDONLY);
    if (fdFrom < 0)
    {
        printf("Error opening file '%s'\n", filenameFrom);
        return;
    }

    int fileSize = getFileSize(filenameFrom);

    int fdShared = open(sharedFilename, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fdShared < 0)
    {
        printf("Error creating shared memory file '%s'\n", sharedFilename);
        close(fdFrom);
        return;
    }

    if (ftruncate(fdShared, fileSize) < 0)
    {
        printf("Error setting shared memory file size\n");
        close(fdFrom);
        close(fdShared);
        return;
    }

    void *src = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fdFrom, 0);
    if (src == MAP_FAILED)
    {
        printf("Error mapping source file\n");
        close(fdFrom);
        close(fdShared);
        return;
    }

    void *dst = mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fdShared, 0);
    if (dst == MAP_FAILED)
    {
        printf("Error mapping shared memory file\n");
        munmap(src, fileSize);
        close(fdFrom);
        close(fdShared);
        return;
    }

    memcpy(dst, src, fileSize);

    munmap(src, fileSize);
    munmap(dst, fileSize);
    close(fdFrom);
    close(fdShared);

    if (!quiet)
        printf("File '%s' copied to shared memory file '%s' using mmap\n", filenameFrom, sharedFilename);
}

void copyFileFromSharedMemoryMmap(char *filenameTo, char *sharedFilename, int fileSize, int quiet)
{
    int fdTo = open(filenameTo, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fdTo < 0)
    {
        printf("Error creating file '%s'\n", filenameTo);
        return;
    }

    int fdShared = open(sharedFilename, O_RDONLY);
    if (fdShared < 0)
    {
        printf("Error opening shared memory file '%s'\n", sharedFilename);
        close(fdTo);
        return;
    }

    void *src = mmap(NULL, fileSize, PROT_READ, MAP_SHARED, fdShared, 0);
    if (src == MAP_FAILED)
    {
        printf("Error mapping shared memory file\n");
        close(fdTo);
        close(fdShared);
        return;
    }

    void *dst = mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (dst == MAP_FAILED)
    {
        printf("Error creating anonymous memory mapping\n");
        munmap(src, fileSize);
        close(fdTo);
        close(fdShared);
        return;
    }

    memcpy(dst, src, fileSize);
    write(fdTo, dst, fileSize);

    munmap(src, fileSize);
    munmap(dst, fileSize);
    close(fdTo);
    close(fdShared);

    if (!quiet)
        printf("File '%s' copied from shared memory file '%s' using mmap\n", filenameTo, sharedFilename);
}

void receiveFileFifo(char *filenameTo, char *fifoName, int quiet)
{
    mkfifo(fifoName, 0666);

    int fdTo = open(filenameTo, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fdTo < 0)
    {
        printf("Error creating file '%s'\n", filenameTo);
        return;
    }

    int fdFifo = open(fifoName, O_RDONLY);
    if (fdFifo < 0)
    {
        printf("Error opening FIFO '%s'\n", fifoName);
        close(fdTo);
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytesRead = 0;
    while ((bytesRead = read(fdFifo, buffer, BUFFER_SIZE)) > 0)
    {
        write(fdTo, buffer, bytesRead);
    }

    close(fdTo);
    close(fdFifo);
    unlink(fifoName);

    if (!quiet)
        printf("File received from FIFO '%s' successfully\n", fifoName);
}

void sendFileFifo(char *filenameFrom, char *fifoName, int quiet)
{
    int fdFrom = open(filenameFrom, O_RDONLY);
    if (fdFrom < 0)
    {
        printf("Error opening file '%s'\n", filenameFrom);
        return;
    }

    mkfifo(fifoName, 0666);

    int fdFifo = open(fifoName, O_WRONLY);
    if (fdFifo < 0)
    {
        printf("Error opening FIFO '%s'\n", fifoName);
        close(fdFrom);
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytesRead = 0;
    while ((bytesRead = read(fdFrom, buffer, BUFFER_SIZE)) > 0)
    {
        write(fdFifo, buffer, bytesRead);
    }

    close(fdFrom);
    close(fdFifo);

    if (!quiet)
        printf("File sent to FIFO '%s' successfully\n", fifoName);
}
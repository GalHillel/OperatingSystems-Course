# Chat Server with Reactor Pattern

This is a chat server implementation that supports an unlimited number of clients using the Reactor pattern.
The server uses a single-threaded event-driven approach to handle multiple client connections efficiently.

## Prerequisites

Before running the chat server, ensure that you have the following software installed:

- GCC (GNU Compiler Collection)
- Linux or Unix-like operating system

## Building the Chat Server

To build the chat server, follow these steps:

1. Open a terminal and navigate to the project directory.
2. Compile the source files using the provided Makefile by running the following command: make all


This command will compile the `react_server.c` file and generate the executable `react_server`.

## Running the Chat Server

To run the chat server, follow these steps:

1. In the terminal, navigate to the project directory (if not already there).
2. Start the server by running the following command: ./react_server

The server will start listening for incoming connections on port 9034.

## Connecting to the Chat Server

To connect to the chat server, you can use a Telnet client or any other client that supports TCP connections. Here's an example of connecting using Telnet:

1. Open a new terminal window.
2. Run the following command to connect to the chat server: telnet localhost 9034

Replace `localhost` with the IP address or hostname of the server if running on a different machine.

3. Once connected, you can start sending messages to the server.

## Chatting with Clients

Once connected to the server, each client can send as many messages as they want without limit. The server will receive and display the messages from the clients. There is no message sending back to the clients in this implementation.

## Stopping the Chat Server

To stop the chat server, you can press `Ctrl + C` in the terminal window where the server is running. This will terminate the server and close all client connections.

## Conclusion

The chat server with the Reactor pattern provides a scalable and efficient solution for handling multiple client connections. Clients can send messages without limits, and the server receives and displays the messages. Feel free to explore and modify the code to suit your needs.

If you have any questions or issues, please don't hesitate to reach out.

Happy chatting!








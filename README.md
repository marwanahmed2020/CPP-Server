# Learning TCP Server

This folder is for learning how a simple TCP server works in C++.
I wrote it in a student way, not a professional way, so it is easier to study and change.

## What this file is

The main file is `server.cpp`.
It is a small practice server that shows the basic ideas behind sockets, clients, rooms, and threads.

## What the program does

- opens a TCP socket
- listens on a port
- accepts client connections
- gives each client a handler function
- keeps simple server data in memory
- has an admin console from the terminal

## Important ideas

### fd
`fd` means file descriptor.
It is just a number from the operating system that represents an open socket.
You use it with `send`, `recv`, and `close`.

### socket
A socket is the thing that lets two computers talk over the network.
In this server, the socket is used for TCP communication.

### thread
A thread is one separate path of execution.
This server can use one thread for each client so many clients can connect at the same time.

### mutex
A mutex is a lock.
It protects shared data when more than one thread wants to use it.

### unordered_map
An `unordered_map` is a hash table.
I used it because it is fast for finding a client or room by key.

### uint16_t
`uint16_t` is a 16-bit unsigned integer.
It is used for port numbers because ports are in the range `0` to `65535`.

### kDefaultPort
This is the default port the server uses if you do not give one on the command line.

### kBacklog
This is the number of waiting connections the operating system allows before the queue becomes full.

## How the parts talk to each other

- `main()` starts the program.
- `run_server()` creates the listening socket.
- `accept()` waits for a client.
- `handle_client()` talks to one client.
- `admin_console()` reads commands from the terminal.
- shared data like clients and rooms is stored in memory and protected by a mutex.

## What is `nc`

`nc` means netcat.
It is a simple terminal tool that can connect to a TCP server.
You can use it to test this server without writing a client program.

Example:

```bash
nc localhost 8080
```

## How to build

From this folder:

```bash
g++ -std=c++17 -pthread -c server.cpp -o server.o
```

## How to think about this project

The server is not finished yet.
It is a learning skeleton.
You are supposed to fill in the missing parts step by step and understand each concept before adding the next one.

## Good next steps

1. implement `send_all`
2. implement `recv_line`
3. make `handle_client` respond to commands
4. add room join and leave logic
5. improve the admin console

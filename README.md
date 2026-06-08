# Hy-Proxy

An asynchronous reverse proxy written in C++ inspired by the architecture of Nginx.

## Features

* HTTP request parsing
* Non-blocking sockets
* Linux `epoll` event loop
* Event-driven state machine
* Asynchronous backend communication
* Reverse proxy request forwarding
* Round-robin load balancing
* Multi-backend support

## Architecture

```text
Client
   |
   v
+---------+
| HyProxy |
+---------+
   |
   +----> Backend 1
   |
   +----> Backend 2
   |
   +----> Backend 3
   |
   +----> Backend N
```

The proxy accepts client connections, parses HTTP requests, selects a backend using a round-robin load balancer, forwards the request, and relays the response back to the client.

All network I/O is handled through a single epoll-based event loop using non-blocking sockets.

## Core Concepts Implemented

* TCP socket programming
* HTTP parsing
* Event-driven architecture
* State machines
* Reverse proxies
* Load balancing
* Linux networking APIs
* Concurrent I/O using epoll

## Connection Lifecycle

```text
READING_REQUEST
        |
        v
CONNECTING_BACKEND
        |
        v
FORWARDING_REQUEST
        |
        v
READING_RESPONSE
        |
        v
FORWARDING_RESPONSE
        |
        v
CLOSED
```

## Load Balancing

Requests are distributed across backend servers using a round-robin algorithm.

Example:

```text
Request 1 -> Backend 3000
Request 2 -> Backend 3001
Request 3 -> Backend 3002
Request 4 -> Backend 3003
Request 5 -> Backend 3004
```

## Performance Test

Five backend servers were configured with a simulated 2-second processing delay.

Results:

```text
20 concurrent requests -> ~8 seconds
100 concurrent requests -> ~40 seconds
```

matching the expected behavior of a round-robin load balancer distributing work across five backends.

## Future Work

* Config file support
* Backend health checks
* Dynamic configuration reload
* Persistent backend connections
* Request metrics and monitoring
* Advanced load-balancing strategies

## Build

```bash
make
```

## Run

```bash
./server
```

## Motivation

This project was built to learn low-level networking, Linux systems programming, asynchronous I/O, and the architecture used by production-grade reverse proxies such as Nginx and HAProxy.

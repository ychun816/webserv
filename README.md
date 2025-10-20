# 🌐 Webserv - HTTP Server in C++98  ♫  [![Spotify](https://img.shields.io/badge/Listen_on-Spotify-1DB954?style=for-the-badge&logo=spotify)](https://open.spotify.com/track/69kOkLUCkxIZYexIgSG8rq)

<p align="center">
  
- Team: [Get Lucky](https://github.com/CrystxlSith/webserv)
- Contributors: [CrystxlSith](https://github.com/CrystxlSith) / [Canybardeloton](https://github.com/Canybardeloton) / [ychun816](https://github.com/ychun816)
- French Original Notes: [CrystxlSith/webserv](https://github.com/CrystxlSith/webserv) 

![Screenshot from 2025-06-11 16-30-04](https://github.com/user-attachments/assets/4839e8e7-7f47-4046-b392-467a044e9a60)

</p>

---

<p align="center">
  <img src="https://img.shields.io/badge/C++-98-FFD4B2?style=flat&logo=cplusplus&logoColor=white" alt="C++98"/> <!-- pastel orange -->
  <img src="https://img.shields.io/badge/HTTP_Server-Implemented-BFEFFF?style=flat&logo=nginx&logoColor=white" alt="HTTP Server"/> <!-- pastel blue -->
  <img src="https://img.shields.io/badge/Non--Blocking_I/O-Learned-FFC4D6?style=flat&logoColor=white" alt="Non-Blocking I/O"/> <!-- pastel pink -->
  <img src="https://img.shields.io/badge/CGI-Implemented-C5A3E8?style=flat&logoColor=white" alt="CGI"/> <!-- pastel purple -->
  <img src="https://img.shields.io/badge/Sockets-Advanced-FFF1A8?style=flat&logoColor=white" alt="Sockets"/> <!-- pastel yellow -->
  <img src="https://img.shields.io/badge/Networking-Learned-D4F4DD?style=flat&logoColor=white" alt="Networking"/> <!-- pastel green -->
</p>

---

## 📑 Table of Contents

- [About / Project Overview](#about--project-overview)
- [Key Features](#key-features)
- [Prerequisites](#prerequisites)
- [Installation & Setup](#installation--setup)
- [Project Structure](#project-structure)
- [Configuration](#configuration)
- [Usage](#usage)
- [Development Phases](#development-phases)
  - [Phase 1: Foundation](#phase-1-foundation)
  - [Phase 2: Core HTTP Protocol](#phase-2-core-http-protocol)
  - [Phase 3: Advanced Features](#phase-3-advanced-features)
- [Technical Constraints](#technical-constraints)
- [I/O Multiplexing Comparison](#io-multiplexing-comparison)
  - [select() vs poll() vs epoll()](#select-vs-poll-vs-epoll)
  - [Read/Write Functions Comparison](#readwrite-functions-comparison)
- [Testing Guide](#testing-guide)
  - [Basic Tests](#basic-tests)
  - [Advanced Tests](#advanced-tests)
  - [Stress Testing](#stress-testing)
- [Resources](#resources)

---

## About / Project Overview

Implementation of an HTTP server in **C++98** capable of handling web requests in a non-blocking manner, similar to **NGINX**.

This project demonstrates:
- 🚀 Building a production-grade HTTP server from scratch
- 🔄 Implementing non-blocking I/O with multiplexing (select/poll/epoll)
- 🌐 Understanding HTTP protocol and web server architecture
- ⚡ Handling concurrent connections efficiently
- 🎯 Writing robust, standards-compliant C++98 code

The server supports modern web features including static file serving, CGI execution, file uploads, custom error pages, and directory listing.

---

## Key Features

- ✅ **Non-blocking HTTP Server** - Handles multiple connections concurrently
- ✅ **HTTP Methods Support** - GET, POST, DELETE
- ✅ **Configurable** - Customizable ports, routes, and server behavior
- ✅ **Static File Handling** - Serves HTML, CSS, JS, images, and other static assets
- ✅ **CGI Support** - Executes PHP, Python, and other CGI scripts
- ✅ **File Upload** - Handles multipart/form-data file uploads
- ✅ **Custom Error Pages** - Personalized 404, 500, and other error responses
- ✅ **Directory Listing** - Auto-generates index pages when enabled
- ✅ **Virtual Hosting** - Multiple server configurations on different ports
- ✅ **HTTP Redirections** - 301, 302 redirects support

---

## Prerequisites

- C++ compiler with C++98 support (g++, clang++)
- Make
- Unix-like system (Linux/MacOS)
- (Optional) PHP-CGI or Python for CGI testing
- (Optional) `curl`, `siege` for testing

---

## Installation & Setup

```bash
# Clone the repository
git clone https://github.com/ychun816/webserv.git
cd webserv

# Compile the project
make

# Run the server with default configuration
./webserv

# Or specify a custom config file
./webserv config/custom.conf
```

---

## Project Structure

```
webserv/
├── srcs/              # Source files
│   ├── Server.cpp     # Main server logic
│   ├── Request.cpp    # HTTP request parser
│   ├── Response.cpp   # HTTP response generator
│   ├── Config.cpp     # Configuration parser
│   └── CGI.cpp        # CGI handler
├── includes/          # Header files
│   ├── Server.hpp
│   ├── Request.hpp
│   ├── Response.hpp
│   └── Config.hpp
├── config/            # Configuration files
│   ├── default.conf   # Default server configuration
│   └── example.conf   # Example configurations
├── www/               # Static web content for testing
│   ├── index.html
│   ├── css/
│   ├── js/
│   └── images/
├── cgi-bin/           # CGI scripts
│   ├── post.py        # Python CGI example
│   └── info.php       # PHP info script
├── Makefile           # Build configuration
└── README.md          # This file
```

---

## Configuration

The server can be configured via a configuration file similar to NGINX syntax:

```nginx
server {
    listen 8080;                    # Port to listen on
    server_name example.com;        # Server name (virtual host)
    root /var/www/html;             # Document root directory

    location / {
        methods GET POST;           # Allowed HTTP methods
        index index.html;           # Default index file
        autoindex on;               # Enable directory listing
    }

    location /uploads {
        methods GET POST DELETE;
        upload_dir /var/www/uploads;
        max_body_size 10M;          # Maximum upload size
    }

    location /cgi-bin {
        methods GET POST;
        cgi_extension .py .php;     # CGI file extensions
        cgi_path /usr/bin/python3;  # CGI interpreter path
    }

    error_page 404 /errors/404.html;
    error_page 500 502 503 504 /errors/50x.html;
}

# Multiple server blocks for virtual hosting
server {
    listen 8081;
    server_name another.com;
    root /var/www/another;
    # ... additional configuration
}
```

### Configuration Options

| Directive | Description | Example |
|-----------|-------------|---------|
| `listen` | Port number to bind | `8080` |
| `server_name` | Virtual host domain | `example.com` |
| `root` | Document root path | `/var/www/html` |
| `methods` | Allowed HTTP methods | `GET POST DELETE` |
| `index` | Default index files | `index.html index.htm` |
| `autoindex` | Enable directory listing | `on` or `off` |
| `upload_dir` | File upload directory | `/uploads` |
| `max_body_size` | Max request body size | `10M` |
| `cgi_extension` | CGI file extensions | `.py .php` |
| `error_page` | Custom error pages | `404 /404.html` |

---

## Server Architecture & Workflow

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                         CLIENT LAYER                             │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐        │
│  │ Browser  │  │  cURL    │  │  Mobile  │  │   API    │        │
│  │  Client  │  │  Client  │  │   App    │  │  Client  │        │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘        │
└───────┼─────────────┼─────────────┼─────────────┼───────────────┘
        │             │             │             │
        └─────────────┴─────────────┴─────────────┘
                      │ HTTP Requests
                      ▼
        ┌─────────────────────────────────┐
        │      NETWORK LAYER (TCP)        │
        │    Socket Binding (0.0.0.0)     │
        │      Port: 8080, 8081...        │
        └────────────────┬────────────────┘
                         │
                         ▼
        ┌─────────────────────────────────┐
        │   I/O MULTIPLEXING LAYER        │
        │  ┌──────────────────────────┐   │
        │  │  select() / poll() /     │   │
        │  │  epoll() Event Loop      │   │
        │  │  (Non-blocking I/O)      │   │
        │  └──────────────────────────┘   │
        └────────────────┬────────────────┘
                         │
                         ▼
        ┌─────────────────────────────────┐
        │    WEBSERV CORE ENGINE          │
        │  ┌─────────────────────────┐    │
        │  │  Connection Manager     │    │
        │  │  Request Parser         │    │
        │  │  Response Generator     │    │
        │  │  Config Handler         │    │
        │  └─────────────────────────┘    │
        └────────────────┬────────────────┘
                         │
        ┌────────────────┴────────────────┐
        │                                 │
        ▼                                 ▼
┌───────────────┐              ┌──────────────────┐
│ STATIC FILES  │              │  DYNAMIC CONTENT │
│   Handler     │              │     Handler      │
│               │              │                  │
│ • HTML/CSS/JS │              │ • CGI Scripts    │
│ • Images      │              │ • PHP/Python     │
│ • Documents   │              │ • File Upload    │
│ • Downloads   │              │ • POST Processing│
└───────────────┘              └──────────────────┘
        │                                 │
        └────────────────┬────────────────┘
                         │
                         ▼
        ┌─────────────────────────────────┐
        │      RESPONSE PIPELINE          │
        │  • Set Headers                  │
        │  • Set Status Code              │
        │  • Send Response                │
        └────────────────┬────────────────┘
                         │
                         ▼
        ┌─────────────────────────────────┐
        │         CLIENT RESPONSE         │
        │    HTTP/1.1 200 OK              │
        │    Content-Type: text/html      │
        │    Connection: keep-alive       │
        └─────────────────────────────────┘
```

---

### Request Processing Flow

```
CLIENT REQUEST                    WEBSERV PROCESSING
─────────────────                ──────────────────────

   ┌─────────┐
   │ Client  │
   │ Sends   │
   │ Request │
   └────┬────┘
        │
        │ GET /index.html HTTP/1.1
        │ Host: localhost:8080
        │ Connection: keep-alive
        │
        ▼
┌──────────────────┐
│  1. ACCEPT()     │ ◄──── New connection arrives
│  New Socket FD   │       on listening socket
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│  2. RECV/READ    │ ◄──── Read data from socket
│  Read Request    │       (non-blocking)
└────────┬─────────┘
         │
         ▼
┌──────────────────────────────────────────┐
│  3. PARSE REQUEST                        │
│  ┌────────────────────────────────────┐  │
│  │ • Parse Request Line               │  │
│  │   Method: GET                      │  │
│  │   URI: /index.html                 │  │
│  │   Version: HTTP/1.1                │  │
│  │                                    │  │
│  │ • Parse Headers                    │  │
│  │   Host: localhost:8080             │  │
│  │   Connection: keep-alive           │  │
│  │   User-Agent: Mozilla/5.0          │  │
│  │                                    │  │
│  │ • Parse Body (if POST/PUT)         │  │
│  └────────────────────────────────────┘  │
└────────┬─────────────────────────────────┘
         │
         ▼
┌──────────────────────────────────────────┐
│  4. ROUTE MATCHING                       │
│  ┌────────────────────────────────────┐  │
│  │ • Match server block (port/host)   │  │
│  │ • Match location block             │  │
│  │ • Check allowed methods            │  │
│  │ • Determine handler type           │  │
│  └────────────────────────────────────┘  │
└────────┬─────────────────────────────────┘
         │
         ├──────────────┬──────────────────┐
         │              │                  │
         ▼              ▼                  ▼
┌─────────────┐  ┌─────────────┐  ┌─────────────┐
│  STATIC     │  │    CGI      │  │   UPLOAD    │
│  FILE       │  │  HANDLER    │  │  HANDLER    │
└──────┬──────┘  └──────┬──────┘  └──────┬──────┘
       │                │                │
       │                │                │
       ▼                ▼                ▼
┌─────────────┐  ┌─────────────┐  ┌─────────────┐
│ • Open file │  │ • Fork()    │  │ • Parse     │
│ • Read      │  │ • Setup ENV │  │   multipart │
│   content   │  │ • Exec CGI  │  │ • Save file │
│ • Detect    │  │ • Pipe I/O  │  │ • Return    │
│   MIME type │  │ • Wait()    │  │   location  │
└──────┬──────┘  └──────┬──────┘  └──────┬──────┘
       │                │                │
       └────────────────┼────────────────┘
                        │
                        ▼
┌──────────────────────────────────────────┐
│  5. BUILD RESPONSE                       │
│  ┌────────────────────────────────────┐  │
│  │ • Set Status Line                  │  │
│  │   HTTP/1.1 200 OK                  │  │
│  │                                    │  │
│  │ • Set Headers                      │  │
│  │   Content-Type: text/html          │  │
│  │   Content-Length: 1234             │  │
│  │   Connection: keep-alive           │  │
│  │   Server: Webserv/1.0              │  │
│  │                                    │  │
│  │ • Attach Body                      │  │
│  │   <!DOCTYPE html><html>...         │  │
│  └────────────────────────────────────┘  │
└────────┬─────────────────────────────────┘
         │
         ▼
┌──────────────────┐
│  6. SEND/WRITE   │ ◄──── Send response to socket
│  Send Response   │       (non-blocking, may need
└────────┬─────────┘       multiple writes)
         │
         ▼
┌──────────────────┐
│  7. KEEP-ALIVE   │ ◄──── Connection management
│  or CLOSE        │       • Keep-alive: wait for next request
└──────────────────┘       • Close: close socket
```

---

### Event Loop (I/O Multiplexing) Flow

```
┌─────────────────────────────────────────────────────────────┐
│                    MAIN EVENT LOOP                          │
│                   (Non-blocking I/O)                        │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
        ┌──────────────────────────────┐
        │  Initialize Server           │
        │  • Create listening socket   │
        │  • Bind to port(s)           │
        │  • Set non-blocking          │
        │  • Listen for connections    │
        └──────────────┬───────────────┘
                       │
                       ▼
        ┌──────────────────────────────┐
        │  Setup I/O Multiplexing      │
        │  • select() / poll() / epoll │
        │  • Add listen socket to set  │
        └──────────────┬───────────────┘
                       │
                       ▼
        ╔══════════════════════════════╗
        ║    INFINITE EVENT LOOP       ║
        ║                              ║
        ║  while (server_running) {    ║
        ╚══════════════╤═══════════════╝
                       │
                       ▼
        ┌──────────────────────────────┐
        │  1. Wait for Events          │
        │  • select()/poll()/epoll()   │
        │  • Timeout: 1 second         │
        │  • Blocks until activity     │
        └──────────────┬───────────────┘
                       │
                       ▼
        ┌──────────────────────────────┐
        │  2. Check for Events         │
        │  • Return value > 0          │
        │  • Loop through ready FDs    │
        └──────────────┬───────────────┘
                       │
        ┌──────────────┴──────────────┐
        │                             │
        ▼                             ▼
┌──────────────────┐        ┌──────────────────┐
│ LISTEN SOCKET    │        │ CLIENT SOCKET    │
│ (New Connection) │        │ (Existing Conn)  │
└────────┬─────────┘        └────────┬─────────┘
         │                           │
         ▼                           ▼
┌──────────────────┐        ┌──────────────────┐
│  accept()        │        │  Check Event     │
│  • Get new FD    │        │  Type            │
│  • Set non-block │        └────────┬─────────┘
│  • Add to FD set │                 │
└──────────────────┘        ┌────────┴─────────┐
                            │                  │
                            ▼                  ▼
                    ┌──────────────┐   ┌──────────────┐
                    │ READABLE     │   │  WRITABLE    │
                    │ (POLLIN)     │   │  (POLLOUT)   │
                    └──────┬───────┘   └──────┬───────┘
                           │                  │
                           ▼                  ▼
                    ┌──────────────┐   ┌──────────────┐
                    │  recv/read   │   │  send/write  │
                    │  data        │   │  response    │
                    └──────┬───────┘   └──────┬───────┘
                           │                  │
                           ▼                  │
                    ┌──────────────┐          │
                    │  Parse       │          │
                    │  Request     │          │
                    └──────┬───────┘          │
                           │                  │
                           ▼                  │
                    ┌──────────────┐          │
                    │  Process     │          │
                    │  Handler     │          │
                    └──────┬───────┘          │
                           │                  │
                           ▼                  │
                    ┌──────────────┐          │
                    │  Build       │          │
                    │  Response    │          │
                    └──────┬───────┘          │
                           │                  │
                           └──────────────────┘
                                   │
                                   ▼
                    ┌──────────────────────────┐
                    │  Response Complete?      │
                    └──────┬──────────┬────────┘
                           │          │
                      YES  │          │  NO
                           ▼          ▼
                    ┌──────────┐  ┌─────────────┐
                    │ Keep     │  │ Keep FD in  │
                    │ Alive?   │  │ write set   │
                    └────┬─────┘  └─────────────┘
                         │
                    ┌────┴────┐
               YES  │         │  NO
                    ▼         ▼
            ┌─────────┐  ┌─────────┐
            │ Wait for│  │ close() │
            │ next    │  │ Remove  │
            │ request │  │ from set│
            └─────────┘  └─────────┘
                    │
                    └──────────┐
                               │
                               ▼
                    ┌──────────────────┐
                    │  3. Cleanup      │
                    │  • Remove closed │
                    │  • Check timeout │
                    └──────┬───────────┘
                           │
                           ▼
                    ┌──────────────────┐
                    │  } // Loop back  │
                    └──────────────────┘
```

---

### CGI Execution Flow

```
CLIENT                  WEBSERV                    CGI SCRIPT
──────                  ───────                    ──────────

  │                         │                           │
  │  POST /cgi-bin/post.py  │                           │
  │  Content-Length: 45     │                           │
  │  username=test&pwd=123  │                           │
  ├────────────────────────►│                           │
  │                         │                           │
  │                         │  1. Parse Request         │
  │                         │  2. Identify CGI Handler  │
  │                         │                           │
  │                         │  3. fork()                │
  │                         │     ┌──────────┐          │
  │                         │     │ Parent   │          │
  │                         │     │ Process  │          │
  │                         │     └────┬─────┘          │
  │                         │          │                │
  │                         │          │ Child          │
  │                         │     ┌────▼─────┐          │
  │                         │     │ Child    │          │
  │                         │     │ Process  │          │
  │                         │     └────┬─────┘          │
  │                         │          │                │
  │                         │  4. Setup Environment     │
  │                         │     Variables:            │
  │                         │     • REQUEST_METHOD=POST │
  │                         │     • CONTENT_LENGTH=45   │
  │                         │     • QUERY_STRING=...    │
  │                         │     • PATH_INFO=/post.py  │
  │                         │          │                │
  │                         │  5. Create Pipes          │
  │                         │     stdin  ─┐             │
  │                         │     stdout ─┤             │
  │                         │             │             │
  │                         │  6. dup2() redirect       │
  │                         │     stdin/stdout to pipes │
  │                         │          │                │
  │                         │  7. execve()              │
  │                         │     /usr/bin/python3      │
  │                         │     /cgi-bin/post.py      │
  │                         ├───────────────────────────►│
  │                         │                           │
  │                         │                   8. CGI Script Runs
  │                         │                      • Read stdin
  │                         │                      • Process data
  │                         │                      • Generate HTML
  │                         │                      • Write to stdout
  │                         │                           │
  │                         │  9. Read from stdout pipe │
  │                         │◄──────────────────────────┤
  │                         │     Content-Type: ...     │
  │                         │     <html>...</html>      │
  │                         │                           │
  │                         │  10. waitpid()            │
  │                         │      (wait for child)     │
  │                         │                           │
  │                         │  11. Parse CGI Output     │
  │                         │      • Headers            │
  │                         │      • Body               │
  │                         │                           │
  │                         │  12. Build HTTP Response  │
  │  HTTP/1.1 200 OK        │                           │
  │  Content-Type: text/html│                           │
  │  <html>...</html>       │                           │
  │◄────────────────────────┤                           │
  │                         │                           │
```

---

## Usage

### Starting the Server

```bash
# Use default configuration
./webserv

# Specify custom configuration file
./webserv config/myserver.conf

# The server will display startup information:
# Server listening on 0.0.0.0:8080
# Server listening on 0.0.0.0:8081
# Webserv started successfully
```

### Basic Testing

```bash
# Test with curl
curl http://localhost:8080

# Test specific endpoint
curl http://localhost:8080/index.html

# Test with browser
# Open http://localhost:8080 in your web browser
```

---

## Development Phases

### Phase 1: Foundation

**Goal**: Set up basic server infrastructure

- ✅ Create Makefile
- ✅ Basic server configuration
- ✅ Implement non-blocking socket
- ✅ Basic connection handling
- ✅ Socket creation and binding
- ✅ Event loop implementation (select/poll/epoll)

**Key Concepts**:
- Socket programming (socket, bind, listen, accept)
- Non-blocking I/O with `fcntl()`
- I/O multiplexing basics

---

### Phase 2: Core HTTP Protocol

**Goal**: Implement HTTP request/response cycle

- ✅ HTTP request parser
- ✅ HTTP response generator
- ✅ Header handling
- ✅ HTTP methods (GET, POST, DELETE)
- ✅ Static file serving
- ✅ MIME type detection

**Key Concepts**:
- HTTP/1.1 protocol specification
- Request line parsing (method, URI, version)
- Header parsing (key-value pairs)
- Status codes (200, 404, 500, etc.)
- Content-Type and other response headers

---

### Phase 3: Advanced Features

**Goal**: Add production-ready features

- ✅ Dynamic configuration parsing
- ✅ Custom error pages
- ✅ CGI support (PHP, Python)
- ✅ File upload handling
- ✅ Directory listing
- ✅ HTTP redirections
- ✅ Virtual hosting (multiple server blocks)
- ✅ Chunked transfer encoding

**Key Concepts**:
- CGI protocol (environment variables, stdin/stdout)
- Multipart/form-data parsing
- Process forking and pipe communication
- Configuration file parsing

---

## Technical Constraints

### Core Requirements

- ⚠️ **High Availability**: The server must remain available under heavy load
- ⚠️ **Non-blocking I/O**: All I/O operations must be non-blocking
- ⚠️ **Error Handling**: Robust error management (no crashes, memory leaks)
- ⚠️ **C++98 Compliance**: Code must conform to C++98 standard
- ⚠️ **No External Libraries**: Cannot use boost, libevent, or similar libraries

### Performance Considerations

- Handle **1000+ concurrent connections**
- Support **100+ requests per second**
- Efficient memory usage (no memory leaks)
- Graceful handling of slow clients
- Timeout management for idle connections

---

## I/O Multiplexing Comparison

### select() vs poll() vs epoll()

| Feature | select() | poll() | epoll() (Linux Only) |
|---------|----------|--------|---------------------|
| **Max FD limit** | 1024 (FD_SETSIZE) | No hard limit (based on system memory) | No hard limit |
| **FD representation** | Bitmask (`fd_set`) | Array of `pollfd` structs | Kernel-managed event list |
| **Performance (many FDs)** | Poor (linear scan) | Better than select() | Excellent (O(1) with `epoll_wait`) |
| **Edge/Level Triggered** | Level-triggered only | Level-triggered only | Supports both edge-triggered and level-triggered |
| **Modifying FDs** | Rebuild entire set each time | Rebuild entire array each time | Add/remove/mod via `epoll_ctl()` |
| **Portability** | POSIX standard (widely supported) | POSIX standard (widely supported) | Linux only |

#### Implementation Notes

- **`select()`** uses a bitmask to track FDs. You must reinitialize it every loop, and it scales poorly with many connections.
- **`poll()`** uses an array of structures, allowing more flexibility, but still rechecks all FDs each time.
- **`epoll()`** is designed for high-performance servers. It maintains an internal kernel list of interested FDs and only returns those ready, making it highly scalable.

---

### Read/Write Functions Comparison

#### English Version

| Function | Description | Usage Context | Special Features / Notes |
|----------|-------------|---------------|-------------------------|
| `read()` | Reads raw bytes from a file descriptor | General (files, sockets, pipes, etc.) | Simple, blocking or non-blocking I/O |
| `recv()` | Reads from a socket, similar to `read()` | Sockets only | Supports flags like `MSG_PEEK`, `MSG_WAITALL` |
| `write()` | Writes raw bytes to a file descriptor | General (files, sockets, pipes, etc.) | Returns number of bytes written (may be partial) |
| `send()` | Writes to a socket, similar to `write()` | Sockets only | Supports flags like `MSG_NOSIGNAL` |

#### 中文版本

| 函式 | 功能說明 | 使用場合 | 特殊功能 / 備註 |
|------|---------|----------|----------------|
| `read()` | 從檔案描述符讀取原始位元資料 | 通用（檔案、socket、管線等） | 簡單,可阻塞或非阻塞 I/O |
| `recv()` | 從 socket 讀取資料,類似 `read()` | 只用於 socket | 支援 `MSG_PEEK`、`MSG_WAITALL` 等旗標 |
| `write()` | 將原始資料寫入檔案描述符 | 通用（檔案、socket、管線等） | 回傳實際寫入位元數,可能是部分寫入 |
| `send()` | 將資料寫入 socket,類似 `write()` | 只用於 socket | 支援 `MSG_NOSIGNAL` 等旗標 |

#### Usage in HTTP Server

- `read()`/`recv()` used to receive HTTP requests from clients
- `write()`/`send()` used to send HTTP responses back
- Must check return values (`0` = connection closed, `-1` = error) and handle `errno` properly
- Non-blocking mode requires handling `EAGAIN` / `EWOULDBLOCK`

---

## Testing Guide

### Basic Tests

#### 1. Simple GET Request
```bash
curl http://localhost:8080
```
Tests basic server responsiveness and HTML serving.

#### 2. Force HTTP/1.0 with Connection Close
```bash
curl -v --http1.0 -H "Connection: close" http://localhost:8080/
```
Checks server compatibility with legacy HTTP/1.0 and explicit connection closure.

#### 3. Custom Host Header (Virtual Hosting)
```bash
curl -v -H "Host: getlucky" http://127.0.0.1:8080/
```
Tests virtual hosting by specifying a non-default Host header.

#### 4. Raw HTTP Request via Netcat
```bash
echo -e "GET / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n" | nc localhost 8080
```
Sends a minimal HTTP GET request manually to inspect raw response behavior.

#### 5. Basic Netcat Connection
```bash
nc localhost 8080
# Then type manually:
# GET / HTTP/1.1
# Host: localhost:8080
# (press Enter twice)
```
Establishes a plain TCP connection to test server responsiveness.

#### 6. Netcat with Timeout (Slow Client Simulation)
```bash
nc -i 5 localhost 8080
```
Similar to above, with a 5-second interval between lines to simulate slow clients.

---

### Advanced Tests

#### 7. POST Request with Form Data
```bash
curl -X POST -d "username=test&password=123" http://localhost:8080/cgi-bin/post.py
```
Tests server's ability to handle POST requests and form data processing via CGI.

#### 8. File Upload Test
```bash
# Generate a large test file
dd if=/dev/zero of=large_file.txt bs=11M count=10

# Upload the file
curl -X POST -F "file=@large_file.txt" http://localhost:8080/upload/
```
Creates a 110MB dummy file for upload testing. Verifies multipart/form-data handling.

#### 9. Download File Using curl
```bash
curl -v -o downloaded_file.txt http://localhost:8080/upload/large_file.txt
```
Verifies file downloading capability from the server.

#### 10. DELETE Request Simulation
```bash
curl -X DELETE http://localhost:8080/upload/large_file.txt
```
Verifies support for the DELETE HTTP method, often used in RESTful APIs.

#### 11. Test Without Method (Malformed Request)
```bash
echo -e " / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n" | nc localhost 8080
```
Sends a malformed request without HTTP method to test error handling.

---

### Stress Testing

#### 12. Siege Stress Test
```bash
siege -b -c 255 -t1M http://127.0.0.1:8080/
```
Runs a high-concurrency benchmark (255 clients for 1 minute) to assess server performance under load.

**Expected Results**:
- No crashes or memory leaks
- Successful handling of concurrent connections
- Response times remain reasonable
- Proper error handling under overload

#### 13. Apache Bench (ab) Test
```bash
ab -n 10000 -c 100 http://localhost:8080/
```
Alternative stress test tool: 10,000 requests with 100 concurrent connections.

#### 14. Load Test with wrk
```bash
wrk -t4 -c200 -d30s http://localhost:8080/
```
Modern HTTP benchmarking tool: 4 threads, 200 connections, 30 seconds duration.

---


## Resources

### HTTP Protocol
- [RFC 2616 - HTTP/1.1](https://www.rfc-editor.org/rfc/rfc2616)
- [RFC 7230-7235 - HTTP/1.1 Updated](https://httpwg.org/specs/)
- [MDN HTTP Documentation](https://developer.mozilla.org/en-US/docs/Web/HTTP)

### Socket Programming
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [The Linux Programming Interface](http://man7.org/tlpi/)
- [Unix Network Programming by Stevens](https://www.amazon.com/Unix-Network-Programming-Volume-Networking/dp/0131411551)

### I/O Multiplexing
- [select() Man Page](https://man7.org/linux/man-pages/man2/select.2.html)
- [poll() Man Page](https://man7.org/linux/man-pages/man2/poll.2.html)
- [epoll() Man Page](https://man7.org/linux/man-pages/man7/epoll.7.html)
- [The C10K Problem](http://www.kegel.com/c10k.html)

### CGI
- [CGI Specification](https://www.rfc-editor.org/rfc/rfc3875)
- [CGI Programming Tutorial](https://www.tutorialspoint.com/python/python_cgi_programming.htm)

### Testing Tools
- [cURL Documentation](https://curl.se/docs/)
- [Siege](https://www.joedog.org/siege-home/)
- [Apache Bench (ab)](https://httpd.apache.org/docs/2.4/programs/ab.html)
- [wrk - Modern HTTP Benchmarking](https://github.com/wg/wrk)

---

<p align="center">
  <i>Built with ❤️ for 42 School</i>
</p>


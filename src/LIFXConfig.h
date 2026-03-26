#pragma once

#define UDP_PORT 56700
//receiving data buffer
#define BUFFER_SIZE 256
#define SOCKET_TIMEOUT_MS 2000
#define MAX_SEND_TRIES 5

//header constants
#define ORIGIN 0
#define ADDRESSABLE true
#define TAGGED false
#define PROTOCOL 1024
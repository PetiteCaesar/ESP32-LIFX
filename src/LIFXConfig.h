#pragma once

#define UDP_PORT 56700
//receiving data buffer
#define BUFFER_SIZE 256
#define MAX_DEVICES 16
#define SOCKET_TIMEOUT_MS 2000
#define MAX_SEND_TRIES 5

//header constants
#define ORIGIN 0
#define ADDRESSABLE 1
#define TAGGED 0
#define PROTOCOL 1024

//Other constants
#define DISCOVER_SOURCE_ID 6767
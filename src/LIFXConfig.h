#pragma once

#define UDP_PORT 56700
//receiving data buffer
#define BUFFER_SIZE 256
//max discoverable devices
#define MAX_DEVICES 16
#define SOCKET_TIMEOUT_MS 2000
//unused
#define MAX_SEND_TRIES 5

//how many get messages can be queued at once (MAX uint8_t)
#define GET_STATE_BUFFER_SIZE 16
// #define USE_RAW_FUNCTIONS


//header constants (ignore)
#define ORIGIN 0
#define ADDRESSABLE 1
#define TAGGED 0
#define PROTOCOL 1024

//Other constants
#define DISCOVER_SOURCE_ID 6767
#define GET_RESPONSE_SOURCE_ID 4747
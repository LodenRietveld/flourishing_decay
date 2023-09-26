#pragma once

#include <Arduino.h>

constexpr int FRAME_RATE = 60;

enum class target_t : unsigned char
{
    RELAY,
    LED,
    BOTH
};

enum class i2c_return_t : unsigned char
{
    SUCCESS = 0,
    WRITE_FAILED,
    READ_FAILED,
    WRITE_BUFFER_TOO_LONG,
    READ_TOO_FEW_BYTES
};


enum class FD_CMD : unsigned char
{
    SET_RELAY = 0x1,
    SET_LED =   0x2,
    SET_BOTH =  0x3,
    FADE_LED =  0x4,
    SET_ALL =   0x8,
    RESET =     0x10, //probably unused
    STOP =      0x11 // probably unused
};


#pragma pack(push, 1)
struct FD_HEADER {
    unsigned short cmd : 6;
    unsigned short idx : 10;
};

struct fd_msg
{
    FD_HEADER header;
    unsigned short value; // split this in 12 bits of value and 4 bits of time to change?
    unsigned short time;
};
#pragma pack(pop, 1)


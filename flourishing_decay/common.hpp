#pragma once

#include <Arduino.h>

constexpr int FRAME_RATE = 60;
constexpr uint8_t NUM_FLOWERS = 10;

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
#pragma pack(pop)

inline String
to_string(FD_CMD cmd)
{
    switch(cmd) {
        case FD_CMD::SET_RELAY:
            return "SET_RELAY";
        case FD_CMD::SET_LED:
            return "SET_LED";
        case FD_CMD::SET_BOTH:
            return "SET_BOTH";
        case FD_CMD::FADE_LED:
            return "FADE_LED";
        case FD_CMD::SET_ALL:
            return "SET_ALL";
        case FD_CMD::RESET:
            return "RESET";
        case FD_CMD::STOP:
            return "STOP";
    }
}

inline String
to_string(fd_msg& msg)
{
    return "{" + to_string((FD_CMD) msg.header.cmd) + ", " + String(msg.header.idx) + " ::::: " + String(msg.value) + ", " + String(msg.time) + "}";
}


struct queue_item {
    bool free;
    fd_msg item;
};

struct msg_queue 
{
    int received_msgs = 0;
    queue_item queue[10];

    msg_queue()
    {
        for (int i = 0; i < 10; i++){
            queue[i].free = true;
        }
    }

    void
    add_msg(uint8_t* raw)
    {
        for (int i = 0; i < 10; i++){
            if (queue[i].free) {
                memcpy(&queue[i].item, raw, sizeof(fd_msg));
                queue[i].free = false;
                ++received_msgs;
                return;
            }
        }
    }

    fd_msg&
    get_msg(int idx)
    {
        return queue[idx].item;
    }

    void
    remove_msg(int idx)
    {
        if (received_msgs > 0) {
            --received_msgs;
            queue[received_msgs].free = true;
        }
    }

    int
    available()
    {
        return received_msgs;
    }
};

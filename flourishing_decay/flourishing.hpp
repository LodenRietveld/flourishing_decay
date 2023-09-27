#pragma once

#include <Arduino.h>
#include "PCA9685_driver.hpp"
#include "common.hpp"

struct relay
{
    uint8_t pin;
    bool state;
    bool new_state;
    bool done;
};

struct fader
{
    uint8_t pin;
    int16_t current_value;
    int16_t change;
    int8_t steps_left;
    bool done;
};

struct message_queue_item
{
    bool free;
    fd_msg msg;
};

class fd_message_handler {
    public:
    fd_message_handler(PCA9685& pca);
    ~fd_message_handler(){};

    i2c_return_t
    add_message(fd_msg& msg);

    i2c_return_t
    add_message(uint8_t* msg);

    void
    handle_queue();

    void
    update();

    void
    begin();

    private:
    PCA9685& pca;
    static constexpr int msg_queue_size = 10;
    message_queue_item msg_queue[msg_queue_size];
    fd_msg message_queue_mem[msg_queue_size];

    fader led_fades[10];
    relay relay_states[10];

    void
    handle_message(fd_msg& msg);

    int
    led_pin_from_msg_index(int idx);

    int
    relay_pin_from_msg_index(int idx);

    void
    set_led(fd_msg& msg);

    void
    set_relay(fd_msg& msg);
};
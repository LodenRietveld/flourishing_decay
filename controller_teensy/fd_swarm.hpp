#pragma once

#include <Wire.h>

#include "/Users/macbook/data/Projects/pepe/Flourishing Decay/code/flourishing_decay/common.hpp"

#define NUM_WORKERS     10
#define I2C_BASE_ADDR   0x40

struct fd_board
{
    uint8_t board_idx;
    uint8_t on_pin;
};


class fd_worker
{
    public:
    fd_worker(TwoWire& w);
    ~fd_worker() = default;

    void
    setup(int board_num, int i2c_addr);

    bool
    send(fd_msg& msg);

    private:
    TwoWire& w;
    uint8_t board_num;
    uint8_t i2c_addr;
    
    fd_msg last_msg;
};

class fd_swarm
{
    public:
    fd_swarm(TwoWire& w);
    ~fd_swarm() = default;

    void
    forward(uint8_t idx, uint16_t value, uint16_t time, target_t target);

    bool
    do_hard_reboot();

    private:
    TwoWire& w;
    fd_worker* workers[NUM_WORKERS];
    int i2c_bus_reset_count = 0;
};


void
fd_test_board_off(fd_swarm& swarm, fd_board& b);

void
fd_test_board_on(fd_swarm& swarm, fd_board& b);

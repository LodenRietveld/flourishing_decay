#include "fd_swarm.hpp"

fd_swarm::fd_swarm(TwoWire& w)
    : w(w)
{
    w.setClock(100000);
    w.begin();
    for (int i = 0; i < NUM_WORKERS; i++)
    {
        workers[i] = new fd_worker(w);
        workers[i]->setup(i, I2C_BASE_ADDR + i);
    }
}

void
fd_swarm::forward(uint8_t idx, uint16_t value, uint16_t time, target_t type)
{
    auto board = idx / NUM_WORKERS;
    fd_msg msg;

    // Serial.println("Forwarding: " + String(idx) + ", " + String(value) + ", " + String(time));

    switch (type) {
        case target_t::RELAY: {
            msg.header.cmd = (uint8_t) FD_CMD::SET_RELAY;
        }

        case target_t::LED: {
            msg.header.cmd = (uint8_t) FD_CMD::SET_LED;
        }

        case target_t::BOTH: {
            msg.header.cmd = (uint8_t) FD_CMD::SET_BOTH;
        }
    }

    msg.header.idx = idx % NUM_WORKERS;
    msg.value = value;
    msg.time = time;

    auto worker = workers[board];
    worker->send(msg);
}

fd_worker::fd_worker(TwoWire& w)
    : w(w)
{

}

void
fd_worker::setup(int _board_num, int _i2c_addr)
{
    board_num = _board_num;
    i2c_addr = _i2c_addr;
}

void
fd_worker::send(fd_msg& msg)
{
    // Serial.println("Sending over i2c");
    last_msg = msg;
    uint8_t* raw = reinterpret_cast<uint8_t*>(&msg);
    w.beginTransmission(i2c_addr);
    w.write(raw, sizeof(fd_msg));
    int ret = w.endTransmission(true);
    if (ret != 0) {
        Serial.println("transmission failed with code " + String(ret));
    }
    // Serial.println("EndTransmission result is " + String(w.endTransmission(true)));
}

void
fd_test_board_off(fd_swarm& swarm, fd_board& b)
{
    swarm.forward((b.board_idx * 10) + b.on_pin, 0, 0, target_t::BOTH);
}

void
fd_test_board_on(fd_swarm& swarm, fd_board& b)
{
    swarm.forward(++b.on_pin + (b.board_idx * 10), 4095, 0, target_t::BOTH);
    
    if (b.on_pin > 9)
        b.on_pin = 0;
}
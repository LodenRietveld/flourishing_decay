#include "fd_swarm.hpp"

fd_swarm::fd_swarm(TwoWire& w)
    : w(w)
{
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
    last_msg = msg;
    uint8_t* raw = reinterpret_cast<uint8_t*>(&msg);
    w.beginTransmission(i2c_addr);
    w.write(raw, sizeof(fd_msg));
    w.endTransmission(true);
}
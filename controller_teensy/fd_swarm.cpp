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
    if (!(idx >= 0 && idx < TOTAL_FLOWERS))
        return;

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
    if (worker->send(msg)) {
        i2c_bus_reset_count = 0;
    } else {
        i2c_bus_reset_count++;
    }
}

bool
fd_swarm::do_hard_reboot()
{
    return i2c_bus_reset_count > 5;
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
cycle_clock(int cnt)
{
    for (int i = 0; i < cnt; i++){
        digitalWrite(19, LOW);
        delayMicroseconds(5);
        digitalWrite(19, HIGH);
        delayMicroseconds(5);
    }
}

bool
reset_bus()
{
    pinMode(18, INPUT);
    digitalWrite(19, HIGH);
    pinMode(19, OUTPUT);

    for (int i = 0; digitalRead(18) == 0 && i < 9; i++){
        cycle_clock(1);
    }

    Wire.end();
    Wire.setClock(100000);
    Wire.begin();
}

bool
fd_worker::send(fd_msg& msg)
{
    // Serial.println("Sending over i2c");
    last_msg = msg;
    uint8_t* raw = reinterpret_cast<uint8_t*>(&msg);
    w.beginTransmission(i2c_addr);
    w.write(raw, sizeof(fd_msg));
    int ret = w.endTransmission(true);
    if (ret != 0) {
        Serial.println("transmission failed with code " + String(ret) + ", address was " + String(msg.header.idx));
        reset_bus();
        return false;
    }
    return true;
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
#include <stdint.h>
#include <Wire.h>

#include "common.hpp"

class I2CReceiver {
    using on_receive_t = void (*)(int);

    public:
    I2CReceiver(uint8_t addr, TwoWire& w)
        : addr(addr), w(w) {};

    i2c_return_t
    begin();

    void
    set_on_receive_cb(on_receive_t cb);


    on_receive_t on_receive;
    private:
    uint8_t addr;
    TwoWire& w;
    volatile uint8_t rx_buf[32];

    static void
    receive_i2c(int size);
};
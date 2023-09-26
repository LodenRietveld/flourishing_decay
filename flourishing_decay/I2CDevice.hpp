#include <Arduino.h>
#include <SoftWire.h>
#include "common.hpp"

void
pulseLed();

void
errno_to_led(int errn);

String
softwire_strerror(uint8_t errno);

class I2CDevice 
{
    public:
    I2CDevice(){};
    I2CDevice(uint8_t addr, SoftWire& wire);

    i2c_return_t
    begin();

    i2c_return_t
    write(uint8_t* tx_buf, int tx_size);

    i2c_return_t
    read(uint8_t* rx_buf, int tx_size);

    i2c_return_t
    write_then_read(uint8_t* tx_buf, int tx_size, uint8_t* rx_buf, int rx_size);

    private:
    uint8_t addr;
    SoftWire& wire;
    bool begun = false;
    static constexpr uint8_t max_buffer_size = 32;
    uint8_t rx_buf[max_buffer_size] = {0};
    uint8_t tx_buf[max_buffer_size] = {0};

    i2c_return_t
    _read(uint8_t* rx_buf, int tx_size, bool stop);
};
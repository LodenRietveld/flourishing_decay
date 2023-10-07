#include "./I2CDevice.hpp"

constexpr uint8_t I2CDevice::max_buffer_size;

String
softwire_strerror(uint8_t errno)
{
    switch(errno) {
        case 0: return "success";
        case 1: 
        case 2: 
        case 3: return "nack";
        case 4: return "timeout";
        default: return "Unknown error";
    }
}

I2CDevice::I2CDevice(uint8_t addr, SoftWire& wire)
    : addr(addr), wire(wire)
{
}

i2c_return_t
I2CDevice::begin()
{
    wire.setDelay_us(40);
    wire.setTxBuffer(tx_buf, max_buffer_size);
    wire.setRxBuffer(rx_buf, max_buffer_size);
    wire.begin();
    begun = true;

    return i2c_return_t::SUCCESS;
}

i2c_return_t
I2CDevice::write(uint8_t* tx_buffer, int size)
{
    if (size > max_buffer_size) {
        return i2c_return_t::WRITE_BUFFER_TOO_LONG;
    }

    wire.beginTransmission(addr);
    if (wire.write(tx_buffer, size) != size) {
        return i2c_return_t::WRITE_FAILED;
    }

    return (i2c_return_t) wire.endTransmission(true);
}

i2c_return_t
I2CDevice::read(uint8_t* rx_buf, int size)
{
    int max_read_size = size > max_buffer_size ? max_buffer_size : size;
    size_t idx = 0;

    while (idx < max_read_size) {
        int read_size = max_read_size - idx;
        bool stop = idx + read_size > max_read_size;

        const auto ret = _read(rx_buf, read_size, stop);
        if (ret != i2c_return_t::SUCCESS) {
            return ret;
        }

        idx += read_size;
    }
    
    return i2c_return_t::SUCCESS;
}

i2c_return_t
I2CDevice::_read(uint8_t* rx_buf, int size, bool stop)
{
    int recv_bytes = wire.requestFrom(addr, size, stop);

    if (recv_bytes != size) {
        return i2c_return_t::READ_TOO_FEW_BYTES;
    }

    for (int i = 0; i < size; i++){
        rx_buf[i] = wire.read();
    }

    return i2c_return_t::SUCCESS;
}

i2c_return_t
I2CDevice::write_then_read(uint8_t* tx_buf, int tx_size, uint8_t* rx_buf, int rx_size)
{
    const auto ret = write(tx_buf, tx_size);
    if (ret != i2c_return_t::SUCCESS) {
        return ret;
    }

    return read(rx_buf, rx_size);
}

void
pulseLed()
{
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    delay(500);
}

void
errno_to_led(int errn)
{
    for (int i = 0; i < 10; i++){
        digitalWrite(i + 3, LOW);
    }
    digitalWrite(errn+3, HIGH);
}
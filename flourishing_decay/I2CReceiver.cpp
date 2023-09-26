#include "I2CReceiver.hpp"

i2c_return_t
I2CReceiver::begin()
{
    w.onReceive(I2CReceiver::on_receive);
    w.begin(addr);   
}

void
I2CReceiver::set_on_receive_cb(I2CReceiver::on_receive_t cb)
{
    this->on_receive = cb;
}
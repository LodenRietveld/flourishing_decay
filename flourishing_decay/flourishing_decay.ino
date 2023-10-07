#include <Wire.h>
#include <AsyncDelay.h>
#include <elapsedMillis.h>

#include "SoftWire.h"
#include "PCA9685_driver.hpp"

#include "flourishing.hpp"

// CHANGE ME TO CHANGE MY I2C ADDRESS
constexpr uint8_t FD_WORKER_I2C_ADDR = 0x41;

constexpr uint8_t I2C_RECV_SDA = 14;
constexpr uint8_t I2C_RECV_SCL = 2;

// constexpr uint8_t FDPCAOE = 0;
// constexpr uint8_t FDSCL = 1;
// constexpr uint8_t FDSDA = 2;

// SoftWire w(FDSDA, FDSCL);
// PCA9685 leds(0x40, w);
fd_message_handler msg_handler;

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

volatile msg_queue i2c_queue;
elapsedMillis timer;

volatile bool i2c_rx = false;

volatile int available;
volatile uint8_t i2c_rx_buf[200];
volatile uint8_t rx_idx = 0;

void
receive_i2c(int size)
{
    uint8_t buf[sizeof(fd_msg)];
    while (Wire.available()) {
        i2c_rx_buf[rx_idx++] = Wire.read();
    }
}

void
copy_rx_buf_to_msg_handler_queue()
{
    uint8_t round = 0;
    while (rx_idx > 0) {
        uint8_t buf[sizeof(fd_msg)];
        for (int i = 0; i < sizeof(fd_msg); i++){
            buf[i] = i2c_rx_buf[i + (round * sizeof(fd_msg))];
        }
        round++;
        rx_idx -= sizeof(fd_msg);
        msg_handler.add_message(buf);
    }
}

void
setup()
{
    Serial.begin(9600);
    Serial.println("Starting up");
    Wire.onReceive(receive_i2c);
    Wire.setPins(I2C_RECV_SDA, I2C_RECV_SCL);
    Wire.setClock(400000);
    Wire.begin(FD_WORKER_I2C_ADDR);
    msg_handler.begin();

    // pinMode(FDPCAOE, OUTPUT);
}

void
loop()
{
    if (rx_idx > 0) {
        copy_rx_buf_to_msg_handler_queue();
        msg_handler.handle_queue();
    }

    if (timer >= 15) {
        msg_handler.update();
        timer = 0;
    }

}
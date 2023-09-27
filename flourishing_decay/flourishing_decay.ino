#include <Wire.h>
#include <AsyncDelay.h>
#include <elapsedMillis.h>

#include "SoftWire.h"
#include "PCA9685_driver.hpp"

#include "flourishing.hpp"

constexpr uint8_t FDPCAOE = 0;
constexpr uint8_t FDSCL = 1;
constexpr uint8_t FDSDA = 2;

constexpr uint8_t PCA_ADDR = 0x40;

SoftWire w(FDSDA, FDSCL);
PCA9685 leds(PCA_ADDR, w);
fd_message_handler msg_handler(leds);

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
setup()
{
    // Serial.begin(9600);
    // Serial.println("Starting up wire");
    Wire.setClock(400000);
    Wire.begin(0x40);
    // Serial.println("Adding onreceive");
    Wire.onReceive(receive_i2c);
    msg_handler.begin();

    pinMode(FDPCAOE, OUTPUT);
}

void
loop()
{
    if (rx_idx > 0) {
        digitalWrite(13, LOW);
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

        msg_handler.handle_queue();
        digitalWrite(13, HIGH);
    }

    if (timer >= 15) {
        msg_handler.update();
        timer = 0;
    }

}
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

    void
    add_msg(uint8_t* raw)
    {
        for (int i = 0; i < 10; i++){
            if (queue[i].free) {
                memcpy(&queue[i].item, raw, sizeof(fd_msg));
                queue[i].free = false;
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

void
receive_i2c(int size)
{
    uint8_t cnt = 0;
    uint8_t buf[sizeof(fd_msg)];
    while (Wire.available() > 1) {
        buf[cnt++] = Wire.read();

        if (cnt == sizeof(fd_msg)) {
            i2c_queue.add_msg(buf);
            i2c_queue.received_msgs++;
            memset(buf, 0, sizeof(fd_msg));
            cnt = 0;
        }
    }
}

uint8_t tx_buf[10] = {0};
void
setup()
{
    // Serial.begin(9600);
    Wire.begin(0x69);
    Wire.onReceive(receive_i2c);
    msg_handler.begin();

    pinMode(FDPCAOE, OUTPUT);
}

void
loop()
{
    if (i2c_queue.available()) {
        for (int i = i2c_queue.received_msgs - 1; i >= 0; i--){
            msg_handler.add_message(i2c_queue.get_msg(i));
            i2c_queue.remove_msg(i);
        }

        msg_handler.handle_queue();
    }

    if (timer >= 15) {
        msg_handler.update();
        timer = 0;
    }
}
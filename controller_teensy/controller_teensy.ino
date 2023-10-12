#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>    

#include <OSCBundle.h>
#include <OSCBoards.h>

#include <Wire.h>

#include "/Users/macbook/data/Projects/pepe/Flourishing Decay/code/flourishing_decay/common.hpp"

#include "fd_swarm.hpp"

constexpr int ETH_RST = 9;
constexpr int QUEUE_SIZE = TOTAL_FLOWERS * 4;

EthernetUDP Udp;
// CHECK THIS
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

IPAddress ip(169, 254, 145, 242);
const unsigned int port = 4200;

fd_swarm swarm(Wire);
uint8_t queue_count = 0;

uint16_t msg_idx = 0;
uint16_t handle_idx = 0;
uint8_t changed_indices[QUEUE_SIZE] = {0};
elapsedMicros queue_time;

enum class fd_state : uint8_t
{
    FLOWER,
    RESET
};

fd_state state = fd_state::RESET;
int fade_time = 4000;
float delay_step = fade_time / float(TOTAL_FLOWERS * 4);
uint8_t reset_idx = 0;
uint8_t reset_to_value = 1;

elapsedMillis queue_monitor_timer;
elapsedMillis reset_timer;


float delay_time_fact[TOTAL_FLOWERS] = {0};

void
queue_change(uint8_t idx, uint8_t value)
{
    //drop message
    if (queue_count >= QUEUE_SIZE)
        return;

    //circular buffer
    if (msg_idx >= QUEUE_SIZE)
        msg_idx = 0;

    uint8_t v_queue = (value & 0x1) << 7;
    v_queue |= idx & 0x7f;
    changed_indices[msg_idx++] = v_queue;
    queue_count++;
    // Serial.println("Message index: " + String(msg_idx));
}

void
handle_queue_item()
{
    if (handle_idx >= QUEUE_SIZE)
        handle_idx = 0;
    
    if (queue_count <= 0)
        return;

    uint8_t v = changed_indices[handle_idx++];
    swarm.forward(v & 0x7f, (v >> 7) * 4095, 2000, target_t::BOTH);
    queue_count--;
}

void
do_reset()
{
    SCB_AIRCR = 0x05FA0004;
    asm volatile("dsb");
}

void
calc_delay_time_fact()
{
    for (int i = 0; i < TOTAL_FLOWERS; i++) {
        delay_time_fact[i] = (cos(TWO_PI * (i / float(TOTAL_FLOWERS))) + 1.) / 2.;
    }
}

void
setup()
{
    Serial.begin(9600);

    if (CrashReport) {
        Serial.print(CrashReport);
    }
    
    pinMode(ETH_RST, OUTPUT);
    digitalWrite(ETH_RST, LOW);
    delay(5);
    digitalWrite(ETH_RST, HIGH);
    delay(20);

    Ethernet.begin(mac, ip);

    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else {
        Serial.println("Ethernet shield found.");
    }

    Udp.begin(port);
    Serial.print("Local ip address is ");
    Ethernet.localIP().printTo(Serial);
    Serial.println();

    calc_delay_time_fact();
}


void
loop()
{
    OSCMessage inc;

    int size = Udp.parsePacket();
    char data_buf[4000];
    size_t idx = 0;

    if (size > 0) {
        // Serial.println("Received udp packet!");
        while (size--) {
            data_buf[idx] = Udp.read();
            inc.fill(data_buf[idx++]);
        }

        if (!inc.hasError()) {
            if (strcmp(inc.getAddress(), "/flower") == 0) {
                uint8_t idx = inc.getInt(0);
                uint8_t v = inc.getInt(1);
                queue_change(idx, v);
            } else if (strcmp(inc.getAddress(), "/reset") == 0) {
                fade_time = inc.getInt(0);
                reset_to_value = inc.getInt(1);
                state = fd_state::RESET;
                delay_step = fade_time / float(TOTAL_FLOWERS * 4);
            } else {
                // Serial.println("Message address was " + String(inc.getAddress()));
            }
        } else {
            // Serial.print("Message error, raw data is ");
            // Serial.println(String(data_buf));
        }
    }

    if (queue_monitor_timer > 500) {
        Serial.println("queue size: " + String(queue_count) + ", msg_idx: " + String(msg_idx) + ", handle_idx: " + String(handle_idx));
        queue_monitor_timer = 0;
    }

    switch(state) {
        case fd_state::FLOWER: {
            if (queue_time > 20) { 
                handle_queue_item();
                queue_time = 0;
            }
            break;
        }

        case fd_state::RESET: {
            if (swarm.do_hard_reboot())
                do_reset();

            if (reset_idx >= TOTAL_FLOWERS) {
                state = fd_state::FLOWER;
                reset_idx = 0;
                // Serial.println("Set state to flower");
            } else {
                swarm.forward(reset_idx, reset_to_value * 4095, 2000, target_t::BOTH);
                // Serial.println("Delay time: " + String(delay_step + (delay_time_fact[reset_idx] * delay_step)));
                delay(delay_step + (delay_time_fact[reset_idx] * delay_step));
                reset_idx++;
            }
            break;
        }
    }
}

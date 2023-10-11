#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>    

#include <OSCBundle.h>
#include <OSCBoards.h>

#include <Wire.h>

#include "/Users/macbook/data/Projects/pepe/Flourishing Decay/code/flourishing_decay/common.hpp"

#include "fd_swarm.hpp"

constexpr int ETH_RST = 9;

EthernetUDP Udp;
// CHECK THIS
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

IPAddress ip(169, 254, 145, 242);
const unsigned int port = 4200;

fd_swarm swarm(Wire);


float delay_time_fact[TOTAL_FLOWERS] = {0};

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
        Serial.println(" ");
        // Serial.print("NVRAM Marker Value = ");
        // Serial.println( NVRAM_UINT32[0] );
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

uint8_t msg_idx = 0;
uint8_t changed_indices[TOTAL_FLOWERS] = {0};
elapsedMillis queue_time;

void
queue_change(uint8_t idx, uint8_t value)
{
    uint8_t v_queue = (value & 0x1) << 7;
    v_queue |= idx & 0x7f;
    changed_indices[msg_idx++] = v_queue;
    // Serial.println("Message index: " + String(msg_idx));
}

void
handle_queue()
{
    switch(msg_idx) {
        case 0: {
            return;
        }
        default: {
            for (int i = 0; i < msg_idx; i++) {
                uint8_t& v = changed_indices[i];
                // Serial.println("Handle " + String(msg_idx) + " msg: " + String((v & 0x7f)) + ", " + String(((v >> 7) * 4095)));
                swarm.forward(v & 0x7f, (v >> 7) * 4095, 2000, target_t::BOTH);
                delayMicroseconds(100);
            }

            msg_idx = 0;
            break;
        }

    }
    
    
}

enum class fd_state : uint8_t
{
    FLOWER,
    RESET
};

fd_state state = fd_state::FLOWER;
int fade_time = 0;
float delay_step = 0;
uint8_t reset_idx = 0;
uint8_t reset_to_value = 0;


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
                delay_step = fade_time / float(TOTAL_FLOWERS * 2);
            } else {
                // Serial.println("Message address was " + String(inc.getAddress()));
            }
        } else {
            // Serial.print("Message error, raw data is ");
            // Serial.println(String(data_buf));
        }
    }

    switch(state) {
        case fd_state::FLOWER: {
            if (queue_time > 2) { 
                handle_queue();
                queue_time = 0;
            }
            break;
        }

        case fd_state::RESET: {
            if (reset_idx > 99) {
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

    // delay(1);
}

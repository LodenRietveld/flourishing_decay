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

//distribute based on load?

void
forward_led(OSCMessage& msg)
{
    auto idx = (uint8_t) msg.getInt(0);
    auto value = (uint16_t) msg.getInt(1);
    auto time = (uint16_t) msg.getInt(2);
    swarm.forward(idx, value, time, target_t::LED);
}


void
forward_relay(OSCMessage& msg)
{
    auto idx = (uint8_t) msg.getInt(0);
    auto value = (uint16_t) msg.getInt(1);
    auto time = (uint16_t) msg.getInt(2);
    swarm.forward(idx, value, time, target_t::RELAY);
}

void
forward_both(OSCMessage& msg)
{
    auto idx = (uint8_t) msg.getInt(0);
    auto value = (uint16_t) msg.getInt(1);
    auto time = 0;
    swarm.forward(idx, value, time, target_t::BOTH);
}


void
setup()
{
    Serial.begin(9600);
    
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
}



void
loop()
{
    OSCMessage inc;

    int size = Udp.parsePacket();
    char data_buf[1000];
    uint8_t idx = 0;

    if (size > 0) {
        Serial.println("Received udp packet!");
        while (size--) {
            data_buf[idx] = Udp.read();
            inc.fill(data_buf[idx++]);
        }

        if (!inc.hasError()) {
            Serial.print("Message reads: ");
            inc.send(Serial);
            Serial.println();
            inc.dispatch("/flower", forward_both);
        } else {
            Serial.print("Message error, raw data is ");
            Serial.println(String(data_buf));
        }
    }

    delay(1);
}

#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>    

#include <OSCBundle.h>
#include <OSCBoards.h>

#include <Wire.h>

#include "/Users/macbook/data/Projects/pepe/Flourishing Decay/code/flourishing_decay/common.hpp"

#include "fd_swarm.hpp"

EthernetUDP Udp;
// CHECK THIS
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

IPAddress ip(128, 32, 122, 252);
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
    auto time = (uint16_t) msg.getInt(2);
    swarm.forward(idx, value, time, target_t::BOTH);
}


void
setup()
{
    Ethernet.begin(mac, ip);
    Udp.begin(port);
}

void
loop()
{
    OSCBundle inc;
    int size = Udp.parsePacket();

    if (size > 0) {
        while (size--) {
            inc.fill(Udp.read());
        }

        if (!inc.hasError()) {
            inc.dispatch("/led", forward_led);
            inc.dispatch("/relay", forward_relay);
            inc.dispatch("/both", forward_both);
        }
    }

    delay(1);
}

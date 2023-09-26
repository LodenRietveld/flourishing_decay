#include "flourishing.hpp"
#include "common.hpp"

static constexpr int fd_message_handler::msg_queue_size;

fd_message_handler::fd_message_handler(PCA9685& pca)
    : pca(pca)
{
}

void
fd_message_handler::begin()
{
    pca.begin();
    for (int i = 0; i < 10; i++){
        auto p = relay_pin_from_msg_index(i);
        pinMode(p, OUTPUT);
        digitalWrite(p, LOW);
    }
}

void
fd_message_handler::add_message(fd_msg& msg)
{
    for (int i = 0; i < fd_message_handler::msg_queue_size; i++){
        if (msg_queue[i].free) {
            *msg_queue[i].msg = msg;
            return;
        }
    }
}

void
fd_message_handler::handle_queue()
{
    for (int i = 0; i < fd_message_handler::msg_queue_size; i++){
        if (!msg_queue[i].free) {
            handle_message(*msg_queue[i].msg);
            msg_queue[i].free = true;
        }
    }
}

void
fd_message_handler::handle_message(fd_msg& msg)
{
    switch ((FD_CMD) msg.header.cmd) {
        //fallthrough
        case FD_CMD::FADE_LED: 
        case FD_CMD::SET_LED: {
            auto& led = led_fades[msg.header.idx];
            int num_steps = msg.time / FRAME_RATE;
            int value_change = msg.value - led.current_value;

            led.pin = led_pin_from_msg_index(msg.header.idx);
            led.done = false;

            if (num_steps == 0) {
                led.change = value_change;
                led.steps_left = 1;
            } else {
                led.change = value_change / num_steps;
                led.steps_left = num_steps;
            }
            break;
        }

        case FD_CMD::SET_RELAY: {
            auto& rel = relay_states[msg.header.idx];
            auto num_frames_to_wait = msg.time / FRAME_RATE;

            rel.pin = relay_pin_from_msg_index(msg.header.idx);
            rel.frames_until_change = num_frames_to_wait;
            rel.new_state = msg.value > 0;
            rel.done = false;
            break;
        }

        default:
            break;
    }
}


void
fd_message_handler::update()
{
    for (int i = 0; i < 10; i++) {
        if (!led_fades[i].done) {
            auto& fade = led_fades[i];
            auto pin = fade.pin;
            auto new_value = fade.current_value + fade.change;

            if (new_value > PCA9685_MAX) {
                fade.current_value = PCA9685_MAX;
                fade.change = 0;
                fade.done = true;
                continue;
            } else if (new_value < PCA9685_MIN) {
                fade.current_value = PCA9685_MIN;
                fade.change = 0;
                fade.done = true;
                continue;
            } else {
                fade.current_value = new_value;
            }

            pca.setPin(pin, new_value);

            if (fade.steps_left-- <= 0) {
                fade.change = 0;
                fade.done = true;
            }
        }

        if (!relay_states[i].done) {
            auto& rel = relay_states[i];
            if (rel.frames_until_change-- <= 0) {
                digitalWrite(rel.pin, rel.new_state);
                rel.state = rel.new_state;
                rel.done = true;
            }
        }
    }
}

int
fd_message_handler::led_pin_from_msg_index(int idx)
{
    return idx;
}

int fd_message_handler::relay_pin_from_msg_index(int idx)
{
    return idx + 3;
}


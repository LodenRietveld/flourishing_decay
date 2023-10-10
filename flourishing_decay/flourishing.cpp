#include "flourishing.hpp"
#include "common.hpp"

static constexpr int fd_message_handler::msg_queue_size;

fd_message_handler::fd_message_handler(PCA9685& pca)
    : pca(pca)
{
    for (int i = 0; i < msg_queue_size; i++){
        msg_queue[i].free = true;
    }
}

void
fd_message_handler::begin()
{
    pca.begin();
    for (int i = 0; i < NUM_FLOWERS; i++){
        auto p = relay_pin_from_msg_index(i);
        pinMode(p, OUTPUT);
        digitalWrite(p, LOW);
    }
}

i2c_return_t
fd_message_handler::add_message(fd_msg& msg)
{
    for (int i = 0; i < fd_message_handler::msg_queue_size; i++){
        if (msg_queue[i].free) {
            msg_queue[i].msg = msg;
            msg_queue[i].free = false;
            return i2c_return_t::SUCCESS;
        }
    }

    return i2c_return_t::WRITE_BUFFER_TOO_LONG;
}

i2c_return_t
fd_message_handler::add_message(uint8_t* msg)
{
    for (int i = 0; i < fd_message_handler::msg_queue_size; i++){
        if (msg_queue[i].free) {
            msg_queue[i].msg = *((fd_msg*) msg);
            msg_queue[i].free = false;
            return i2c_return_t::SUCCESS;
        }
    }

    return i2c_return_t::WRITE_BUFFER_TOO_LONG;
}

void
fd_message_handler::handle_queue()
{
    for (int i = 0; i < fd_message_handler::msg_queue_size; i++){
        if (!msg_queue[i].free) {

            handle_message(msg_queue[i].msg);
            msg_queue[i].free = true;
        }
    }
}

void
fd_message_handler::set_led(fd_msg& msg)
{
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
    // Serial.println("Changing led " + String(led.pin) + " in " + String(num_steps) + " steps with " + String(led.change) + " per step to " + String(msg.value));
}

void
fd_message_handler::set_relay(fd_msg& msg)
{
    auto& rel = relay_states[msg.header.idx];

    rel.pin = relay_pin_from_msg_index(msg.header.idx);
    rel.new_state = msg.value > 0;
    rel.done = false;
}

void
fd_message_handler::handle_message(fd_msg& msg)
{
    switch ((FD_CMD) msg.header.cmd) {
        case FD_CMD::SET_BOTH: {
            set_led(msg);
            set_relay(msg);
            break;
        }

        case FD_CMD::FADE_LED: 
        case FD_CMD::SET_LED: {
            set_led(msg);
            break;
        }

        case FD_CMD::SET_RELAY: {
            set_relay(msg);
        }

        default:
            break;
    }
}


void
fd_message_handler::update()
{
    for (int i = 0; i < NUM_FLOWERS; i++) {
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
            digitalWrite(rel.pin, rel.new_state);
            rel.state = rel.new_state;
            rel.done = true;
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

bool
has_idx(uint8_t* arr, uint8_t idx)
{
    for (int i = 0; i < NUM_FLOWERS; i++){
        if (arr[i] == idx) {
            return true;
        }
    }
    
    return false;
}

fd_message_handler::startup_routine()
{
    uint8_t idx[NUM_FLOWERS] = {0};
    uint8_t idcs[NUM_FLOWERS];
    for (int i = 0; i < NUM_FLOWERS; i++) {
        idcs[i] = i;
    }

    for (int i = 0; i < NUM_FLOWERS; i++) {
        uint8_t new_idx;
        do {
            new_idx = idcs[rand() % NUM_FLOWERS];
            idx[i] = new_idx;
        }
        while (has_idx(idx, new_idx));
    }

    for (int i = 0; i < NUM_FLOWERS; i++){
        fd_msg msg;
        msg.header.cmd = FD_CMD::SET_BOTH;
        msg.header.idx = idx[i];
        msg.time = 500*i;
        msg.value = 4095;

        this->handle_message(msg);
    }
}
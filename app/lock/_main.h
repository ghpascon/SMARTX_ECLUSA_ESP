#pragma once
#include <AbleButtons.h>

// ==== PIN DEFINITIONS ====
#define PIN_SENSOR_1 18
#define PIN_SENSOR_2 17

#define BUTTON_PIN_1 16
#define BUTTON_PIN_2 15

#define PIN_LOCK_1 40
#define PIN_LOCK_2 39

// ==== CLASS DEFINITION ====
class LOCK
{
public:
    LOCK(uint8_t sensorPin, uint8_t lockPin, uint8_t buttonPin)
        : sensor(sensorPin),
          button(buttonPin),
          pin_sensor(sensorPin),
          pin_lock(lockPin),
          pin_button(buttonPin)
    {
    }

    bool is_open = false;        // physical door open/closed state
    bool button_pressed = false; // button physical pressed state

    void setup()
    {
        sensor.setDebounceTime(50);
        button.setDebounceTime(50);

        pinMode(pin_sensor, INPUT_PULLUP);
        pinMode(pin_button, INPUT_PULLUP);
        pinMode(pin_lock, OUTPUT);
    }

    void loop()
    {
        sensor.handle();
        button.handle();

        update_sensor();
        update_button();
        update_outputs();
    }

    void trigger_open()
    {
        _open_door = true;
    }

private:
    AblePullupButton sensor;
    AblePullupButton button;
    uint8_t pin_sensor;
    uint8_t pin_lock;
    uint8_t pin_button;

    bool _open_door = false;
    bool ensure_close = false;
    unsigned long current_time = 0;

    void update_sensor()
    {
        bool new_state = sensor.isPressed();
        if (new_state != is_open)
            is_open = new_state;
    }

    void update_button()
    {
        bool new_state = button.isPressed();
        if (new_state != button_pressed)
            button_pressed = new_state;
    }

    void update_outputs()
    {
        if (_open_door)
        {
            ensure_close = true;
            if (current_time == 0)
                current_time = millis();
        }
        else if (!is_open && ensure_close)
        {
            ensure_close = false;
        }

        digitalWrite(pin_lock, ensure_close ? HIGH : LOW);

        const unsigned long pulse_time = 1000;
        if (_open_door && millis() - current_time > pulse_time)
        {
            _open_door = false;
            current_time = 0;
        }
    }
};

// ==== INSTANTIATE TWO LOCKS ====
LOCK lock1(PIN_SENSOR_1, PIN_LOCK_1, BUTTON_PIN_1);
LOCK lock2(PIN_SENSOR_2, PIN_LOCK_2, BUTTON_PIN_2);

// ==== FUNCTIONS ====

bool lock_state()
{
    return lock1.is_open || lock2.is_open;
}

void open_door(int id)
{
    if (id == 1 && !lock2.is_open)
        lock1.trigger_open();
    else if (id == 2 && !lock1.is_open)
        lock2.trigger_open();
}

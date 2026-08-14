#pragma once
#include <AbleButtons.h>

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

    bool is_open = true;         // startup: considera aberta ate o primeiro GND no sensor
    bool button_pressed = false; // button physical pressed state

    void setup()
    {
        sensor.setDebounceTime(50);
        button.setDebounceTime(50);

        pinMode(pin_sensor, INPUT_PULLUP);
        pinMode(pin_button, INPUT_PULLUP);
        pinMode(pin_lock, OUTPUT);
        set_relay(false); // startup: rele desativado em LOW
    }

    void loop()
    {
        if (emergency_override)
        {
            apply_emergency_override();
            return;
        }

        sensor.handle();
        button.handle();

        update_sensor();
        update_button();
        update_outputs();
    }

    bool trigger_open()
    {
        if (emergency_override || is_open)
            return false;

        // Abertura e por comando: nao depende de sensor voltar para HIGH.
        last_open_ms = millis();
        is_open = true;
        return true;
    }

    void set_emergency_override(bool active)
    {
        if (emergency_override == active)
            return;

        emergency_override = active;
        // Ao entrar/sair de EMG, considera estado da porta como aberto/indefinido
        // ate o sensor fechar em GND.
        is_open = true;
        last_open_ms = 0;

        if (active)
        {
            // EMG ativo: trava logica normal e mantem rele desativado em LOW.
            button_pressed = false;
        }
    }

private:
    AblePullupButton sensor;
    AblePullupButton button;
    uint8_t pin_sensor;
    uint8_t pin_lock;
    uint8_t pin_button;

    bool emergency_override = false;
    unsigned long last_open_ms = 0;

    void apply_emergency_override()
    {
        set_relay(false);
    }

    void set_relay(bool active)
    {
        digitalWrite(pin_lock, active ? LOCK_OUTPUT_ACTIVE_LEVEL : LOCK_OUTPUT_INACTIVE_LEVEL);
    }

    void update_sensor()
    {
        const bool sensor_closed = INVERT_SENSOR_LOGIC ? !sensor.isPressed() : sensor.isPressed();
        if (!sensor_closed || !is_open)
            return;

        // Guarda curta evita fechar instantaneamente por leitura residual apos trigger.
        const unsigned long close_guard_ms = 200;
        if (millis() - last_open_ms < close_guard_ms)
            return;

        is_open = false;
    }

    void update_button()
    {
        bool new_state = button.isPressed();
        if (new_state != button_pressed)
            button_pressed = new_state;
    }

    void update_outputs()
    {
        set_relay(!is_open);
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

bool open_door(int id)
{
    if (emg_active)
        return false;

    if (lock1.is_open || lock2.is_open)
        return false;

    if (id == 1)
        return lock1.trigger_open();

    if (id == 2)
        return lock2.trigger_open();

    return false;
}

#include "../../pins.h"
#include "../../vars.h"

class LockVerifications
{
public:
    void setup()
    {
        pinMode(EMG_PIN, INPUT_PULLUP);
        pinMode(PIN_R700, OUTPUT);
    }
    void loop()
    {
        check_emg();
        check_buttons();
        update_state();
        send_door_status();
    }

private:
    void check_emg()
    {
        const bool active_now = digitalRead(EMG_PIN) == EMG_ACTIVE_LEVEL;

        static bool last_emg_state = false;

        emg_active = active_now;
        lock1.set_emergency_override(active_now);
        lock2.set_emergency_override(active_now);

        if (active_now != last_emg_state)
        {
            last_emg_state = active_now;
            write_data("#emg:" + String(active_now ? "1" : "0"));
        }
    }

    void check_buttons()
    {
        static bool last_lock1_pressed = false;
        static bool last_lock2_pressed = false;

        const bool lock1_just_pressed = lock1.button_pressed && !last_lock1_pressed;
        const bool lock2_just_pressed = lock2.button_pressed && !last_lock2_pressed;

        last_lock1_pressed = lock1.button_pressed;
        last_lock2_pressed = lock2.button_pressed;

        const bool any_open = lock1.is_open || lock2.is_open;

        if (lock1_just_pressed)
        {
            if (any_open)
            {
                command_error("BUTTON", "DOOR_IS_OPEN");
                return;
            }
            lock1.trigger_open();
            return;
        }

        if (lock2_just_pressed)
        {
            if (any_open)
            {
                command_error("BUTTON", "DOOR_IS_OPEN");
                return;
            }
            lock2.trigger_open();
        }
    }

    void update_state()
    {
        static bool last_is_open = false;
        bool current_open = lock_state();
        if (last_is_open != current_open)
        {
            last_is_open = current_open;
            write_data("#lock:" + String(current_open ? "1" : "0"));
            digitalWrite(PIN_R700, current_open ? HIGH : LOW);
        }
    }

    void send_door_status()
    {
        static bool last_lock1_state = false;
        static bool last_lock2_state = false;

        // Check Lock 1
        if (lock1.is_open != last_lock1_state)
        {
            last_lock1_state = lock1.is_open;
            if (lock1.is_open)
            {
                write_data("#open:1");
            }
            else
            {
                write_data("#close:1");
            }
        }

        // Check Lock 2
        if (lock2.is_open != last_lock2_state)
        {
            last_lock2_state = lock2.is_open;
            if (lock2.is_open)
            {
                write_data("#open:2");
            }
            else
            {
                write_data("#close:2");
            }
        }
    }
};

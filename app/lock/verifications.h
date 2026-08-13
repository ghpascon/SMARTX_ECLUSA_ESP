#define PIN_R700 41

class LockVerifications
{
public:
    void setup()
    {
        pinMode(PIN_R700, OUTPUT);
    }
    void loop()
    {
        check_buttons();
        update_state();
        send_door_status();
    }

private:
    void check_buttons()
    {
        if (!lock2.is_open && lock1.button_pressed)
            lock1.trigger_open();

        if (!lock1.is_open && lock2.button_pressed)
            lock2.trigger_open();
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

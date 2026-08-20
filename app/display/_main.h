#include "vars.h"
#include "screens.h"

class LCM_DISPLAY
{
public:
    void setup()
    {
        Lcm.begin();
        Lcm.changeBacklight(100);
    }

    void loop()
    {
        load_screen_on_start();
        check_connection();
        write_app_string();
        check_authorized();
    }

private:
    void load_screen_on_start()
    {
        const int load_time = 5000;
        static bool loaded = false;
        if (loaded)
            return;

        Lcm.changePicId(load_screen);

        if (millis() > load_time)
        {
            Lcm.changePicId(is_connected ? app_screen : offline_screen);
            loaded = true;
        }
    }

    void check_connection()
    {
        const int time = 1000;
        static unsigned long current_time = 0;
        if (millis() - current_time < time)
            return;
        current_time = millis();

        static bool last_connected = false;
        if (is_connected == last_connected)
            return;
        last_connected = is_connected;
        Lcm.changePicId(is_connected ? app_screen : offline_screen);
    }

    void write_app_string()
    {
        const int time = 500;
        static unsigned long current_time = 0;
        if (millis() - current_time < time)
            return;
        current_time = millis();
        String msg = "";
        if (!lock1.is_open && !lock2.is_open)
            msg = "Portas Fechadas";
        else if (lock1.is_open && !lock2.is_open)
            msg = "Porta 1 Aberta";
        else if (!lock1.is_open && lock2.is_open)
            msg = "Porta 2 Aberta";
        else
            msg = "Portas Abertas";
        msg += "\nTags lidas: " + String(tags_qty);
        msg += "\n" + extra_msg;
        app_string.write(msg);
    }

    void check_authorized()
    {
        const int time = 3000;
        static unsigned long current_time = 0;
        if (authorized == 0)
        {
            current_time = millis();
            return;
        }
        if (millis() - current_time > time)
        {
            Lcm.changePicId(is_connected ? app_screen : offline_screen);
            authorized = 0;
            identification = "";
            return;
        }
        if (authorized == 1)
            Lcm.changePicId(authorized_screen);
        else if (authorized == 2)
            Lcm.changePicId(denied_screen);
        else
            return;

        String msg = identification;
        identification_string.write(msg);
    }
};

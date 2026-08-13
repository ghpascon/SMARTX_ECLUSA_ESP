
class TELNET
{
protected:
    WiFiServer telnetServer{23};
    WiFiClient telnetClient;

public:
    void config_telnet()
    {
        telnetServer.begin();
        telnetServer.setNoDelay(true);
        Serial.println("Telnet server iniciado na porta 23");
    }

    // Aceita novo cliente se necessário
    void maintain_client()
    {
        if (!telnetClient || !telnetClient.connected())
        {
            WiFiClient newClient = telnetServer.available();
            if (newClient)
            {
                if (telnetClient && telnetClient.connected())
                {
                    telnetClient.stop();
                }
                telnetClient = newClient;
                telnetClient.setNoDelay(true);
                Serial.println("Cliente Telnet conectado");
            }
        }
    }

    bool telnet_write(const String &msg, bool newline = true)
    {
        maintain_client();
        if (!telnetClient || !telnetClient.connected())
            return false;

        Serial.println("Telnet Write: " + msg);

        if (newline)
            return telnetClient.println(msg) > 0;
        return telnetClient.print(msg) > 0;
    }

    // Verifica se chegou algo no telnet; lê até '\n' ou até timeout_ms e faz um print no Serial
    void check_telnet(uint32_t timeout_ms = 200)
    {
        maintain_client();

        if (!telnetClient || !telnetClient.connected())
            return;
        if (!telnetClient.available())
            return;

        unsigned long start = millis();
        String line;

        while (millis() - start < timeout_ms)
        {
            while (telnetClient.available())
            {
                char c = (char)telnetClient.read();
                if (c == '\r')
                    continue; // ignora CR
                if (c == '\n')
                {
                    // imprime a linha completa quando encontrar newline
                    on_receive(line);
                    return;
                }
                line += c;
                // Limita tamanho para evitar consumo excessivo de memória
            }
        }

        if (line.length() > 0)
        {
            on_receive(line);
        }
    }

    long last_receive = 0;
    void check_is_connected()
    {
        is_connected = millis() - last_receive <= 5000;
    }

private:
    void on_receive(String cmd)
    {
        last_receive = millis();
        cmd.toLowerCase();

        if (cmd == "#ping")
            return;

        else if (cmd.startsWith("#open:"))
        {
            int sep = cmd.indexOf(':');
            String doorStr = cmd.substring(sep + 1);
            int door = doorStr.toInt();

            if (door == 1)
                open_door(1);
            else if (door == 2)
                open_door(2);

            telnet_write("#OPEN:" + doorStr);
        }

        else if (cmd.startsWith("#authorized:") || cmd.startsWith("#denied:"))
        {
            int firstSep = cmd.indexOf(':');
            int secondSep = cmd.indexOf(':', firstSep + 1);

            String doorStr = "";
            String card_id = "";

            if (secondSep > 0)
            {
                doorStr = cmd.substring(firstSep + 1, secondSep);
                card_id = cmd.substring(secondSep + 1);
            }
            else
            {
                card_id = cmd.substring(firstSep + 1);
            }

            identification = card_id;
            authorized = cmd.startsWith("#authorized:") ? 1 : 2;

            // Handle multiple doors
            int door = doorStr.toInt();
            if (door == 1 && authorized == 1)
                open_door(1);
            else if (door == 2 && authorized == 1)
                open_door(2);

            telnet_write(authorized == 1 ? "#AUTHORIZED" : "#DENIED");
            telnet_write("DOOR: " + doorStr);
            telnet_write("CARD: " + card_id);
        }

        else if (cmd.startsWith("#tags_qty:"))
        {
            int sep = cmd.indexOf(':');
            tags_qty = (sep > 0) ? cmd.substring(sep + 1).toInt() : 0;
            telnet_write("#TAGS_QTY: " + String(tags_qty));
        }
        else
        {
            telnet_write("INVALID CMD: " + cmd);
        }
    }
};

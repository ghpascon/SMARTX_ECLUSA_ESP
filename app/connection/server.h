#pragma once

class CONNECTION_SERVER
{
protected:
    bool server_started = false;

    void config_server()
    {
        if (server_started)
            return;

        server_connection();
        server_update_firmware();
        server_update_fs();
        server.begin();
        server_started = true;
    }

private:
    void server_connection()
    {
        server.on("/connection", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
            StaticJsonDocument<192> doc;
            doc["is_connected"] = eth_connected ? "connected" : "disconnected";
            doc["ssid"] = ssid;
            doc["ip"] = eth_ip;
            doc["eth_state"] = eth_state;

            String response;
            serializeJson(doc, response);
            request->send(200, "application/json", response); });
    }

    void server_update_firmware()
    {
        server.on(
            "/update_firmware",
            HTTP_POST,
            [](AsyncWebServerRequest *request)
            {
                const bool ok = !Update.hasError();
                request->send(ok ? 200 : 500,
                              "text/plain",
                              ok ? "Firmware atualizado com sucesso. Reiniciando..."
                                 : "Falha na atualizacao de firmware.");

                if (ok)
                    Serial.println("Firmware update finished successfully");
                else
                    Serial.println("Firmware update failed");
            },
            [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
            {
                (void)request;

                if (index == 0)
                {
                    Serial.printf("Firmware upload started: %s\n", filename.c_str());
                    if (!Update.begin(UPDATE_SIZE_UNKNOWN))
                    {
                        Update.printError(Serial);
                    }
                }

                if (len > 0)
                {
                    if (Update.write(data, len) != len)
                    {
                        Update.printError(Serial);
                    }
                }

                if (final)
                {
                    if (!Update.end(true))
                    {
                        Update.printError(Serial);
                    }
                }
            });
    }

    void server_update_fs()
    {
        server.on(
            "/update_fs",
            HTTP_POST,
            [](AsyncWebServerRequest *request)
            {
                const bool ok = !Update.hasError();
                request->send(ok ? 200 : 500, "text/plain", ok ? "Sucesso" : "Falha");

                if (ok)
                    ESP.restart();
            },
            [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
            {
                (void)request;

                if (index == 0)
                {
                    Serial.printf("FS upload started: %s\n", filename.c_str());
                    if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_SPIFFS, 0x290000))
                    {
                        Update.printError(Serial);
                    }
                }

                if (len > 0)
                {
                    if (Update.write(data, len) != len)
                    {
                        Update.printError(Serial);
                    }
                }

                if (final)
                {
                    if (!Update.end(true))
                    {
                        Update.printError(Serial);
                    }
                }
            });
    }
};

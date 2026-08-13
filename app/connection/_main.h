#include "vars.h"
#include "server.h"
#include "eth_pins.h"
#include "eth_callback.h"
#include "telnet.h"

class CONNECTION : private SERVER, public TELNET
{
public:
    void setup()
    {
        config_ap();
        config_server();

        // config ethernet
        WiFi.onEvent(WiFiEvent);

#ifdef ETH_POWER_PIN
        pinMode(ETH_POWER_PIN, OUTPUT);
        digitalWrite(ETH_POWER_PIN, HIGH);
#endif

#if CONFIG_IDF_TARGET_ESP32
        if (!ETH.begin(ETH_TYPE, ETH_ADDR, ETH_MDC_PIN,
                       ETH_MDIO_PIN, ETH_RESET_PIN, ETH_CLK_MODE))
        {
            Serial.println("ETH start Failed!");
        }
        else
        {
            // IP estático 192.168.1.x usando último byte do MAC (com proteção); hostname = SSID do AP
            uint8_t last_octet = (uint8_t)(chipid & 0xFF);
            if (last_octet == 0 || last_octet == 1 || last_octet == 255)
            {
                last_octet = 2; // evita conflito: rede (0), gateway (1), broadcast (255)
            }
            IPAddress local_ip(192, 168, 1, last_octet);
            IPAddress gateway(192, 168, 1, 1);
            IPAddress subnet(255, 255, 255, 0);
            ETH.config(local_ip, gateway, subnet);
            ETH.setHostname(ssid.c_str());
        }
#else
        if (!ETH.begin(ETH_PHY_W5500, 1, ETH_CS_PIN, ETH_INT_PIN, ETH_RST_PIN,
                       SPI3_HOST,
                       ETH_SCLK_PIN, ETH_MISO_PIN, ETH_MOSI_PIN))
        {
            Serial.println("ETH start Failed!");
        }
        else
        {
            // IP estático 192.168.1.x usando último byte do MAC (com proteção); hostname = SSID do AP
            uint8_t last_octet = (uint8_t)(chipid & 0xFF);
            if (last_octet == 0 || last_octet == 1 || last_octet == 255)
            {
                last_octet = 2;
            }
            IPAddress local_ip(192, 168, 1, last_octet);
            IPAddress gateway(192, 168, 1, 1);
            IPAddress subnet(255, 255, 255, 0);
            ETH.config(local_ip, gateway, subnet);
            ETH.setHostname(ssid.c_str());
        }
#endif

        config_telnet();
    }

    void loop()
    {
        check_telnet();
        check_is_connected();
    }

private:
    void config_ap()
    {
        // access point
        chipid = ESP.getEfuseMac(); // Obtém o MAC único
        char id_str[13];
        sprintf(id_str, "%012llX", chipid); // Converte para string hexadecimal
        ssid = "SMTX-" + String(id_str);    // Cria o SSID
        const char *password = "123456789";
        WiFi.softAP(ssid.c_str(), password);
    }
};

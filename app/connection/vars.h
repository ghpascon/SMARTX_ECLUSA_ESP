#pragma once

#if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 0, 0)
#include <ETHClass2.h> // Is to use the modified ETHClass
#define ETH ETH2
#else
#include <ETH.h>
#endif

#include <SPI.h>
#include <WiFi.h>

#include <HTTPClient.h>
#include <ESPmDNS.h>

#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>

String ssid = "";
uint64_t chipid = 0;

bool eth_connected = false;
bool wifi_connected = false;
String eth_state = "stopped";
String eth_ip = "";

bool dhcp_on = false;
String static_ip = "";
String gateway_ip = "";
String subnet_mask = "";

AsyncWebServer server(80);

inline String get_esp_name()
{
    if (ssid.length() > 0)
        return ssid;

    chipid = ESP.getEfuseMac();

    char id_str[13];
    snprintf(id_str, sizeof(id_str), "%012llX", (unsigned long long)chipid);
    ssid = "SMTX-" + String(id_str);

    return ssid;
}

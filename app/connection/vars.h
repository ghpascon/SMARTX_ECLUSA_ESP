#if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 0, 0)
#include <ETHClass2.h> //Is to use the modified ETHClass
#define ETH ETH2
#else
#include <ETH.h>
#endif
#include <SPI.h>
#include <WiFi.h>

#include <HTTPClient.h>
String ssid = "";
uint64_t chipid;
bool eth_connected = false;
String eth_ip = "";

#include <ESPAsyncWebServer.h>
AsyncWebServer server(80);

#include <ArduinoJson.h>

#include <AsyncTCP.h>
#include <Update.h>

extern int tags_qty;
extern String identification;
extern int authorized;
extern bool is_connected;
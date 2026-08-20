#pragma once

#include <Arduino.h>

int tags_qty = 0;
String identification = "";
int authorized = 0;
bool is_connected = false;
bool emg_active = false;
unsigned long last_close = 0;
unsigned long last_send = 0;
String extra_msg = "";
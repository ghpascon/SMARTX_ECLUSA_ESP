#pragma once

#define ETH_MISO_PIN 21
#define ETH_MOSI_PIN 35
#define ETH_SCLK_PIN 45
#define ETH_CS_PIN 47
#define ETH_INT_PIN -1
#define ETH_RST_PIN -1
#define ETH_ADDR 1

#define SD_MISO_PIN 5
#define SD_MOSI_PIN 6
#define SD_SCLK_PIN 7
#define SD_CS_PIN 42

#ifndef ETH_SPI_HOST
#define ETH_SPI_HOST SPI3_HOST
#endif

// Locks and local buttons
#define PIN_SENSOR_1 16
#define PIN_SENSOR_2 17
#ifndef INVERT_SENSOR_LOGIC
#define INVERT_SENSOR_LOGIC true
#endif
#define BUTTON_PIN_1 11
#define BUTTON_PIN_2 12
#define PIN_LOCK_1 1
#define PIN_LOCK_2 2
#define PIN_R700 38

// Wiegand readers
#define PIN_D0_1 5
#define PIN_D1_1 6
#define PIN_D0_2 7
#define PIN_D1_2 15

#define EMG_PIN 18
#define RGB_DATA_PIN 48

#ifndef EMG_ACTIVE_LEVEL
#define EMG_ACTIVE_LEVEL HIGH
#endif

#ifndef LOCK_OUTPUT_ACTIVE_LEVEL
#define LOCK_OUTPUT_ACTIVE_LEVEL LOW
#endif

#ifndef LOCK_OUTPUT_INACTIVE_LEVEL
#define LOCK_OUTPUT_INACTIVE_LEVEL HIGH
#endif

#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wnarrowing"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include "pins.h"

#include "vars.h"

#include <Arduino.h>
#include "esp_system.h"
#include "esp_task_wdt.h"

#include "app/lock/_main.h"
extern LOCK lock1;
extern LOCK lock2;

#include "app/connection/_main.h"
CONNECTION connection;
#include "USB.h"
USBCDC my_usb;

#include "helpers.h"

#include "app/wiegand/_main.h"
extern WIEGAND_READER wiegand1;
extern WIEGAND_READER wiegand2;

#include "app/rgb/_main.h"
LED_RGB led_rgb;

#include "app/display/_main.h"
LCM_DISPLAY lcm_display;

#include "app/lock/verifications.h"
LockVerifications lock_verifications;

#define WATCHDOG_TIMEOUT 30

#include "cmd_handler.h"

// Handle da task
TaskHandle_t taskWiegandHandle;

void taskWiegand(void *pvParameters)
{
  unsigned long lastMillis = 0;

  while (true)
  {
    connection.loop();
    process_stream_commands(Serial, serial_command_buffer, serial_last_byte_ms, "serial");
    process_stream_commands(my_usb, usb_command_buffer, usb_last_byte_ms, "myusb");
    update_command_connection_state();
    lcm_display.loop();
    lock1.loop();
    lock2.loop();
    lock_verifications.loop();
    led_rgb.loop(emg_active, lock1.is_open, lock2.is_open);
    vTaskDelay(pdMS_TO_TICKS(2));
  }
}

void setup()
{
  // Forca estado inativo dos reles logo no boot para evitar acionamento na partida.
  pinMode(PIN_LOCK_1, OUTPUT);
  pinMode(PIN_LOCK_2, OUTPUT);

  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, DISPLAY_RX_PIN, DISPLAY_TX_PIN);
  USB.VID(0x0001);
  USB.PID(0x0003);
  USB.manufacturerName("Smartx");
  USB.productName("ECLUSA");
  USB.usbAttributes(0x80);
  USB.begin();
  my_usb.begin(115200);

  // Inicializa lock cedo para manter saida estavel antes dos modulos de rede.
  lock1.setup();
  lock2.setup();

  connection.setup();
  wiegand1.setup();
  wiegand2.setup();
  led_rgb.setup();
  lcm_display.setup();
  lock_verifications.setup();

  // Cria a task no Core 0
  xTaskCreatePinnedToCore(
      taskWiegand,        // função da task
      "taskWiegand",      // nome
      4096,               // stack size
      NULL,               // parâmetro
      1,                  // prioridade
      &taskWiegandHandle, // handle
      0                   // core 0
  );

  // Configure the Watchdog for both cores
  esp_task_wdt_config_t wdt_config = {
      .timeout_ms = WATCHDOG_TIMEOUT * 1000,
      .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
      .trigger_panic = true};
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);
}

void loop()
{
  esp_task_wdt_reset();

  wiegand1.loop();
  wiegand2.loop();
}

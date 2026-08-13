#include "vars.h"

#include <Arduino.h>
#include "esp_system.h"
#include "esp_task_wdt.h"

#include "app/lock/_main.h"
extern LOCK lock1;
extern LOCK lock2;

#include "app/connection/_main.h"
CONNECTION connection;

#include "helpers.h"

#include "app/wiegand/_main.h"
extern WIEGAND_READER wiegand1;
extern WIEGAND_READER wiegand2;

#include "app/display/_main.h"
LCM_DISPLAY lcm_display;

#include "app/lock/verifications.h"
LockVerifications lock_verifications;

#define WATCHDOG_TIMEOUT 10

// Handle da task
TaskHandle_t taskWiegandHandle;

void taskWiegand(void *pvParameters)
{
  unsigned long lastMillis = 0;

  while (true)
  {
    connection.loop();
    lcm_display.loop();
    lock1.loop();
    lock2.loop();
    lock_verifications.loop();
    vTaskDelay(pdMS_TO_TICKS(2));
  }
}

void setup()
{
  Serial.begin(115200);

  connection.setup();
  wiegand1.setup();
  wiegand2.setup();
  lcm_display.setup();
  lock1.setup();
  lock2.setup();
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

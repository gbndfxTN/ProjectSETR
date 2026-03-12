#include <Arduino.h>
#include "global_data.h"
#include "sensor_manager.h"
#include "display_manager.h"
#include "config.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <WiFi.h>
#include "esp_task_wdt.h"


void task_sensors(void*) {
  sensor_init();
  for (;;) {
    sensor_read_all();
    vTaskDelay(pdMS_TO_TICKS(DHT_DELAY));  // 2 s
  }
}

void task_display(void*) {
  display_init();
  for (;;) {
    display_update();
    vTaskDelay(pdMS_TO_TICKS(1000 / FPS));  // 30 fps
  }
}


void setup() {
  Serial.begin(115200);
  printf("Setup completed.\n");

  // Initialiser les mutex des données globales
  g_data.init();
  printf("Global data mutex initialized\n");

  // Configuration WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);
  printf("Connexion WiFi...\n");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    printf(".");
    fflush(stdout);
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    printf("\nWiFi connecté!\n");
    printf("IP: %s\n", WiFi.localIP().toString().c_str());
  
    esp_task_wdt_delete(xTaskGetIdleTaskHandleForCPU(0));
    esp_task_wdt_delete(xTaskGetIdleTaskHandleForCPU(1));

    // Core 1 : Display + Capteurs (local, temps réel)
    xTaskCreatePinnedToCore(task_display,   "Display",   8192,  NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(task_sensors,   "Sensors",   4096,  NULL, 1, NULL, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    printf("Tasks created on Core 1\n");
  } else {
    printf("\nErreur de connexion WiFi\n");
}
}


void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}


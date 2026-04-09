#include <Arduino.h>
#include "global_data.h"
#include "sensor_manager.h"
#include "sensor_data.h"
#include "display_manager.h"
#include "config.h"
#include "firebase_uploader.h"
#include <stdio.h>
#include <freertos/queue.h>

enum class SensorMessageType : uint8_t {
  Dht,
  Remote
};

struct SensorMessage {
  SensorMessageType type;
  float value1;
  float value2;
  bool flag;
};

QueueHandle_t g_sensor_queue = nullptr;

void task_producer_dht(void*) {
  sensor_init();
  for (;;) {
    float temperature = 0.0f;
    float humidity = 0.0f;

    if (sensor_read_dht(temperature, humidity) && g_sensor_queue != nullptr) {
      SensorMessage msg{};
      msg.type = SensorMessageType::Dht;
      msg.value1 = temperature;
      msg.value2 = humidity;
      msg.flag = false;
      xQueueSend(g_sensor_queue, &msg, pdMS_TO_TICKS(20));
    }

    vTaskDelay(pdMS_TO_TICKS(DHT_DELAY));
  }
}

void task_producer_rs232(void*) {
#if SIMULATE_REMOTE_ESP
  static const float sim_co2_uart_values[] = {520.0f, 610.0f, 750.0f, 940.0f, 680.0f};
  static const float sim_co2_pwm_values[] = {505.0f, 625.0f, 735.0f, 960.0f, 665.0f};
  static const bool sim_presence_values[] = {false, true, true, true, false};
  const size_t sim_count = sizeof(sim_co2_uart_values) / sizeof(sim_co2_uart_values[0]);
  size_t sim_idx = 0;

  printf("[SIM] Producteur distant simule active\n");
  for (;;) {
    SensorMessage msg{};
    msg.type = SensorMessageType::Remote;
    msg.value1 = sim_co2_uart_values[sim_idx];
    msg.value2 = sim_co2_pwm_values[sim_idx];
    msg.flag = sim_presence_values[sim_idx];

    if (g_sensor_queue != nullptr) {
      xQueueSend(g_sensor_queue, &msg, pdMS_TO_TICKS(20));
    }

    sim_idx = (sim_idx + 1) % sim_count;
    vTaskDelay(pdMS_TO_TICKS(REMOTE_SIM_DELAY_MS));
  }
#else
  sensor_uart_init();
  for (;;) {
    float co2_ppm_uart = 0.0f;
    float co2_ppm_pwm = 0.0f;
    bool presence = false;

    if (sensor_uart_read_remote(co2_ppm_uart, co2_ppm_pwm, presence) && g_sensor_queue != nullptr) {
      SensorMessage msg{};
      msg.type = SensorMessageType::Remote;
      msg.value1 = co2_ppm_uart;
      msg.value2 = co2_ppm_pwm;
      msg.flag = presence;
      xQueueSend(g_sensor_queue, &msg, pdMS_TO_TICKS(20));
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
#endif
}

void task_consumer_display(void*) {
  display_init();

  TickType_t last_wake = xTaskGetTickCount();
  for (;;) {
    SensorMessage msg{};
    while (g_sensor_queue != nullptr && xQueueReceive(g_sensor_queue, &msg, 0) == pdTRUE) {
      if (msg.type == SensorMessageType::Dht) {
        sensor_data_set_dht(msg.value1, msg.value2);
      } else if (msg.type == SensorMessageType::Remote) {
        sensor_data_set_remote(msg.value1, msg.value2, msg.flag);
      }
    }

    display_update();
    vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(1000 / FPS));
  }
}


void setup() {
  Serial.begin(115200);
  printf("Setup OLED ESP32: temperature/humidity + RS232\n");

  // Initialiser les mutex des données globales
  g_data.init();
  printf("Global data mutex initialized\n");

  g_sensor_queue = xQueueCreate(SENSOR_QUEUE_LENGTH, sizeof(SensorMessage));
  if (g_sensor_queue == nullptr) {
    printf("Erreur: queue capteurs non creee\n");
    return;
  }

  firebase_uploader_init();

  xTaskCreatePinnedToCore(task_consumer_display, "DisplayConsumer", 8192, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(task_producer_dht, "DhtProducer", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(task_producer_rs232, "UartProducer", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(firebase_uploader_task, "FirebaseUploader", 8192, NULL, 1, NULL, 0);
  vTaskDelay(pdMS_TO_TICKS(100));
  printf("Tasks initialisees (2 producteurs / 1 consommateur / 1 uploader Firebase)\n");
}


void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}

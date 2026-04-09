#include "sensor_data.h"

bool sensor_data_get(SensorData& data) {
  if (xSemaphoreTake(g_data.sensors_mutex, pdMS_TO_TICKS(200))) {
    data = g_data.sensors;
    xSemaphoreGive(g_data.sensors_mutex);
    return true;
  }
  return false;
}

bool sensor_data_set_dht(float temperature, float humidity) {
  if (xSemaphoreTake(g_data.sensors_mutex, pdMS_TO_TICKS(200))) {
    g_data.sensors.temperature = temperature;
    g_data.sensors.humidity = humidity;
    g_data.sensors.last_update_dht = millis();
    xSemaphoreGive(g_data.sensors_mutex);
    return true;
  }
  return false;
}

bool sensor_data_set_remote(float co2_ppm_uart, float co2_ppm_pwm, bool presence_detected) {
  if (xSemaphoreTake(g_data.sensors_mutex, pdMS_TO_TICKS(200))) {
    g_data.sensors.co2_ppm_uart = co2_ppm_uart;
    g_data.sensors.co2_ppm_pwm = co2_ppm_pwm;
    g_data.sensors.presence_detected = presence_detected;
    g_data.sensors.last_update_remote = millis();
    xSemaphoreGive(g_data.sensors_mutex);
    return true;
  }
  return false;
}

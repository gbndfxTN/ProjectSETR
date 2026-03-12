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

bool sensor_data_set_gas(float gas_resistance) {
  if (xSemaphoreTake(g_data.sensors_mutex, pdMS_TO_TICKS(200))) {
    g_data.sensors.gas_resistance = gas_resistance;
    g_data.sensors.last_update_gas = millis();
    xSemaphoreGive(g_data.sensors_mutex);
    return true;
  }
  return false;
}

bool sensor_data_set_ultrason(float distance) {
  if (xSemaphoreTake(g_data.sensors_mutex, pdMS_TO_TICKS(200))) {
    g_data.sensors.distance = distance;
    g_data.sensors.last_update_ultrason = millis();
    xSemaphoreGive(g_data.sensors_mutex);
    return true;
  }
  return false;
}

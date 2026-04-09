#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include "global_data.h"

bool sensor_data_get(SensorData &data);
bool sensor_data_set_dht(float temperature, float humidity);
bool sensor_data_set_remote(float co2_ppm_uart, float co2_ppm_pwm, bool presence_detected);

#endif
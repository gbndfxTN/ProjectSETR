#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include "global_data.h"

bool sensor_data_get(SensorData &data);
bool sensor_data_set_dht(float temperature, float humidity);
bool sensor_data_set_gas(float gas_resistance);
bool sensor_data_set_ultrason(float distance);

#endif
#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <DHT.h>
#include "global_data.h"
#include "config.h"


// Initialisation des capteurs

void sensor_init();

// Lire le capteur local temperature / humidite
bool sensor_read_dht(float &temperature, float &humidity);

// Initialiser et lire la liaison RS232 venant de l'autre ESP32
void sensor_uart_init();
bool sensor_uart_read_remote(float &co2_ppm, bool &presence_detected);

// Trame attendue (ASCII): "CO2:<ppm>;PRES:<0|1>\n"

#endif

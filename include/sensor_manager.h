#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <DHT.h>
#include "global_data.h"
#include "config.h"


// Initialisation des capteurs

void sensor_init();

// Lire les capteurs et mettre à jour les données globales
void sensor_read_dht();
void sensor_read_gas();
void sensor_read_ultrason();

// Lire tous les capteurs

void sensor_read_all();

#endif

#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#include <Arduino.h>
#include "config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Structure pour les données des capteurs
struct SensorData {
  // DHT
  float temperature;
  float humidity;
  
  // Gaz
  float gas_resistance;
  
  // Ultrasonic
  float distance;
  
  // Timestamps
  unsigned long last_update_dht;
  unsigned long last_update_gas;
  unsigned long last_update_ultrason;
};

// Structure pour les données d'affichage
struct DisplayData {
  char line1[OLED_MAX_CHARS_PER_LINE + 1];
  char line2[OLED_MAX_CHARS_PER_LINE + 1];
  char line3[OLED_MAX_CHARS_PER_LINE + 1];
  char line4[OLED_MAX_CHARS_PER_LINE + 1];
    unsigned long last_update;          
};

// Structure globale pour toutes les données
struct GlobalData {
  SensorData sensors;
  DisplayData display;
  
  // Mutex pour protéger l'accès aux données
  SemaphoreHandle_t sensors_mutex;
  SemaphoreHandle_t display_mutex;
  
  // Initialiser les mutex
  void init();
};


// Instance globale
extern GlobalData g_data;

#endif
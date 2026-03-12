#include "global_data.h"

// Instance globale
GlobalData g_data;

// Initialiser tous les mutex
void GlobalData::init() {
  sensors_mutex   = xSemaphoreCreateMutex();
  display_mutex   = xSemaphoreCreateMutex();
}
#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H
#include "config.h"
#include "global_data.h"
#include <time.h>
#include <string.h>
#include <stdio.h>

// Initialisation de l'écran OLED
void display_init();

// Met à jour l'affichage avec les données des capteurs
void display_update();

#endif


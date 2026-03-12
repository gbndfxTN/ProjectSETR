#ifndef DISPLAY_DATA_H
#define DISPLAY_DATA_H

#include "global_data.h"
#include "config.h"

bool display_data_get(DisplayData &data);
void display_data_set(const char *line1, const char *line2, const char *line3, const char *line4);

#endif
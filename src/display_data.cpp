#include "display_data.h"
#include "config.h"
#include <string.h>

namespace {
const char *safe_text(const char *text) {
    return (text != nullptr) ? text : "";
}
}

bool display_data_get(DisplayData &data) {
    if (xSemaphoreTake(g_data.display_mutex, pdMS_TO_TICKS(200))) {
        data = g_data.display;
        xSemaphoreGive(g_data.display_mutex);
        return true;
    }
    return false;
}

void display_data_set(const char *line1, const char *line2, const char *line3, const char *line4) {
    if (xSemaphoreTake(g_data.display_mutex, pdMS_TO_TICKS(200))) {
        strncpy(g_data.display.line1, safe_text(line1), OLED_MAX_CHARS_PER_LINE);
        strncpy(g_data.display.line2, safe_text(line2), OLED_MAX_CHARS_PER_LINE);
        strncpy(g_data.display.line3, safe_text(line3), OLED_MAX_CHARS_PER_LINE);
        strncpy(g_data.display.line4, safe_text(line4), OLED_MAX_CHARS_PER_LINE);
        g_data.display.line1[OLED_MAX_CHARS_PER_LINE] = '\0';
        g_data.display.line2[OLED_MAX_CHARS_PER_LINE] = '\0';
        g_data.display.line3[OLED_MAX_CHARS_PER_LINE] = '\0';
        g_data.display.line4[OLED_MAX_CHARS_PER_LINE] = '\0';
        g_data.display.last_update = millis();
        xSemaphoreGive(g_data.display_mutex);
    }
}
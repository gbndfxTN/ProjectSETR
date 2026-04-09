#include "display_manager.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <string.h>
#include <stdio.h>

#include "sensor_data.h"

namespace {
Adafruit_SSD1306 oled(OLED_WIDTH_PX, OLED_HEIGHT_PX, &Wire, OLED_RESET_PIN);
bool oled_ready = false;
bool screen_blank = false;

const char *safe_text(const char *text) {
	return (text != nullptr) ? text : "";
}

void print_line(uint8_t y, const char *text) {
	char buffer[OLED_MAX_CHARS_PER_LINE + 1];
	strncpy(buffer, safe_text(text), OLED_MAX_CHARS_PER_LINE);
	buffer[OLED_MAX_CHARS_PER_LINE] = '\0';

	oled.setCursor(0, y);
	oled.print(buffer);
}

bool sync_is_active() {
	#if USE_EXTERNAL_SYNC
	return digitalRead(SYNC_DISPLAY_PIN) == SYNC_DISPLAY_ACTIVE_LEVEL;
	#else
	return true;
	#endif
}
}

void display_init() {
	Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
	#if USE_EXTERNAL_SYNC
	pinMode(SYNC_DISPLAY_PIN, INPUT);
	#endif

	if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
		Serial.println("[OLED] Erreur initialisation SSD1306");
		oled_ready = false;
		return;
	}

	oled_ready = true;
	oled.clearDisplay();
	oled.setTextColor(SSD1306_WHITE);
	oled.setTextSize(OLED_TEXT_SIZE);
	oled.setTextWrap(false);

	oled.setCursor(0, 0);
	oled.println("OLED pret");
	oled.display();
}

void display_update() {
	if (!oled_ready) {
		return;
	}

	SensorData data{};
	if (!sensor_data_get(data)) {
		return;
	}

	if (!data.presence_detected) {
		if (!screen_blank) {
			oled.clearDisplay();
			oled.display();
			screen_blank = true;
		}
		return;
	}
	screen_blank = false;

	oled.clearDisplay();
	char line1[OLED_MAX_CHARS_PER_LINE + 1];
	char line2[OLED_MAX_CHARS_PER_LINE + 1];
	char line3[OLED_MAX_CHARS_PER_LINE + 1];
	char line4[OLED_MAX_CHARS_PER_LINE + 1];

	snprintf(line1, sizeof(line1), "Temperature: %.1fC", data.temperature);
	snprintf(line2, sizeof(line2), "Humidite: %.1f%%", data.humidity);
	snprintf(line3, sizeof(line3), "CO2 PWM: %.0f ppm", data.co2_ppm_pwm);
	snprintf(line4, sizeof(line4), "CO2 UART: %.0f ppm", data.co2_ppm_uart);

	print_line(0 * OLED_LINE_HEIGHT, line1);
	print_line(1 * OLED_LINE_HEIGHT, line2);
	print_line(2 * OLED_LINE_HEIGHT, line3);
	print_line(3 * OLED_LINE_HEIGHT, line4);

	oled.display();
}

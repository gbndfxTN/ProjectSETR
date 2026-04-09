#include "firebase_uploader.h"

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <string.h>

#include "config.h"
#include "sensor_data.h"

namespace {
constexpr TickType_t FIREBASE_TASK_DELAY_MS = 500;
constexpr size_t FIREBASE_URL_BUFFER_SIZE = 192;
constexpr size_t FIREBASE_BODY_BUFFER_SIZE = 256;

unsigned long g_last_wifi_attempt_ms = 0;
unsigned long g_last_current_upload_ms = 0;
unsigned long g_last_history_upload_ms = 0;
uint8_t g_history_slot = 0;
bool g_wifi_connecting = false;
bool g_wifi_was_connected = false;

void start_wifi_connect() {
  const unsigned long now = millis();
  if (g_wifi_connecting && (now - g_last_wifi_attempt_ms) < WIFI_RECONNECT_INTERVAL_MS) {
    return;
  }

  g_last_wifi_attempt_ms = now;
  g_wifi_connecting = true;
  WiFi.disconnect(false, true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  printf("[FIREBASE] Connexion Wi-Fi en cours sur %s\n", WIFI_SSID);
}

bool ensure_wifi_connected() {
  const wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    if (!g_wifi_was_connected) {
      printf("[FIREBASE] Wi-Fi connecte, IP=%s\n", WiFi.localIP().toString().c_str());
    }
    g_wifi_connecting = false;
    g_wifi_was_connected = true;
    return true;
  }

  if (g_wifi_was_connected) {
    printf("[FIREBASE] Wi-Fi perdu, reprise automatique des que possible\n");
    g_wifi_was_connected = false;
  }

  if ((millis() - g_last_wifi_attempt_ms) >= WIFI_RECONNECT_INTERVAL_MS || !g_wifi_connecting) {
    start_wifi_connect();
  }
  return false;
}

bool has_complete_data(const SensorData &data) {
  return data.last_update_dht != 0UL && data.last_update_remote != 0UL;
}

void build_payload(const SensorData &data, char *buffer, size_t buffer_size) {
  snprintf(
      buffer,
      buffer_size,
      "{\"temperature\":%.2f,\"humidity\":%.2f,\"co2_ppm_uart\":%.2f,"
      "\"co2_ppm_pwm\":%.2f,\"presence\":%s,\"updatedAt\":{\".sv\":\"timestamp\"}}",
      data.temperature,
      data.humidity,
      data.co2_ppm_uart,
      data.co2_ppm_pwm,
      data.presence_detected ? "true" : "false");
}

bool send_request(const char *method, const char *url, const char *payload) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  if (!http.begin(client, url)) {
    printf("[FIREBASE] Erreur ouverture HTTP: %s\n", url);
    return false;
  }

  http.setTimeout(FIREBASE_HTTP_TIMEOUT_MS);
  http.addHeader("Content-Type", "application/json");

  const int status = http.sendRequest(method, reinterpret_cast<const uint8_t *>(payload), strlen(payload));
  if (status <= 0) {
    printf("[FIREBASE] %s %s a echoue: %s\n", method, url, http.errorToString(status).c_str());
    http.end();
    return false;
  }

  if (status < 200 || status >= 300) {
    printf("[FIREBASE] %s %s retourne HTTP %d\n", method, url, status);
    http.end();
    return false;
  }

  http.end();
  return true;
}

bool upload_current(const SensorData &data) {
  char url[FIREBASE_URL_BUFFER_SIZE];
  char payload[FIREBASE_BODY_BUFFER_SIZE];

  snprintf(
      url,
      sizeof(url),
      "%s/devices/%s/current.json",
      FIREBASE_DATABASE_URL,
      FIREBASE_DEVICE_ID);
  build_payload(data, payload, sizeof(payload));
  return send_request("PATCH", url, payload);
}

bool upload_history(const SensorData &data, uint8_t slot) {
  char url[FIREBASE_URL_BUFFER_SIZE];
  char payload[FIREBASE_BODY_BUFFER_SIZE];

  snprintf(
      url,
      sizeof(url),
      "%s/devices/%s/history/%02u.json",
      FIREBASE_DATABASE_URL,
      FIREBASE_DEVICE_ID,
      static_cast<unsigned>(slot));
  build_payload(data, payload, sizeof(payload));
  return send_request("PUT", url, payload);
}
}

void firebase_uploader_init() {
  WiFi.mode(WIFI_STA);
  start_wifi_connect();
}

void firebase_uploader_task(void *) {
  for (;;) {
    const bool wifi_connected = ensure_wifi_connected();

    SensorData snapshot{};
    if (!wifi_connected || !sensor_data_get(snapshot) || !has_complete_data(snapshot)) {
      vTaskDelay(pdMS_TO_TICKS(FIREBASE_TASK_DELAY_MS));
      continue;
    }

    const unsigned long now = millis();

    if (g_last_current_upload_ms == 0UL ||
        (now - g_last_current_upload_ms) >= FIREBASE_CURRENT_INTERVAL_MS) {
      if (upload_current(snapshot)) {
        g_last_current_upload_ms = now;
      }
    }

    if (g_last_history_upload_ms == 0UL ||
        (now - g_last_history_upload_ms) >= FIREBASE_HISTORY_INTERVAL_MS) {
      if (upload_history(snapshot, g_history_slot)) {
        g_last_history_upload_ms = now;
        g_history_slot = static_cast<uint8_t>((g_history_slot + 1U) % 60U);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(FIREBASE_TASK_DELAY_MS));
  }
}

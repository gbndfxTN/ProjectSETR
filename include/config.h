#ifndef CONFIG_H
#define CONFIG_H

//WIFI
#define WIFI_SSID "Wokwi-GUEST"      // WiFi simulé par Wokwi
#define WIFI_PASSWORD ""
//#define WIFI_SSID "TON_SSID"              
//#define WIFI_PASSWORD "TON_MOT_DE_PASSE" 

//Serveur MQTT
#define MQTT_SERVER   "192.168.1.24"
#define MQTT_PORT     1883
#define MQTT_USER     "gbndfx"
#define MQTT_PASSWORD "?Nala08200!!"

//PINS capteurs
#define DHT_PIN 15
#define DHT_TYPE DHT22
#define DHT_DELAY 2000

// RS232 (liaison avec l'autre ESP32)
#define RS232_BAUD 9600
#define RS232_RX_PIN 26
#define RS232_TX_PIN 27

// Simulation de l'autre ESP32 (CO2 + presence)
// 1 = simule les donnees distantes sans carte externe
// 0 = lit les donnees reelles sur la liaison RS232
#define SIMULATE_REMOTE_ESP 0
#define REMOTE_SIM_DELAY_MS 1000

// Synchronisation externe de l'affichage
#define SYNC_DISPLAY_PIN 17
#define SYNC_DISPLAY_ACTIVE_LEVEL HIGH
// 0 = affichage toujours actif (recommande pour debug/simulation)
// 1 = affichage pilote par le pin de synchro externe
#define USE_EXTERNAL_SYNC 0

// Producteurs multiples / consommateur unique
#define SENSOR_QUEUE_LENGTH 16

//PINS écran
#define OLED_SDA_PIN 5
#define OLED_SCL_PIN 4
#define OLED_I2C_ADDRESS 0x3C
#define OLED_WIDTH_PX 128
#define OLED_HEIGHT_PX 64
#define OLED_RESET_PIN -1
#define OLED_TEXT_SIZE 1
#define OLED_LINE_HEIGHT 16
#define OLED_MAX_CHARS_PER_LINE 21
#define FPS 30
#define BRIGHTNESS 128

#endif

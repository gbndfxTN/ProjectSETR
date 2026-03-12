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
#define DHT_PIN 4
#define DHT_TYPE DHT22
#define DHT_DELAY 2000
#define GAS_PIN 34
#define GAS_DELAY 2000
#define ULTRASON_TRIG_PIN 5
#define ULTRASON_ECHO_PIN 18
#define ULTRASON_DELAY 2000
#define ULTRASON_DETECTION_CM 90.0f

//PINS écran
#define OLED_SDA_PIN 21
#define OLED_SCL_PIN 22
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

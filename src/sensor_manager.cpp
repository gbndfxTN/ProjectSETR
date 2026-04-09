#include "sensor_manager.h"
#include <stdlib.h>
#include <string.h>

// Instance du DHT
DHT dht(DHT_PIN, DHT_TYPE);
HardwareSerial &rs232 = Serial2;

namespace {
constexpr size_t RX_BUFFER_SIZE = 64;
char rx_line[RX_BUFFER_SIZE];
size_t rx_len = 0;

bool parse_remote_frame(const char *line, float &co2_ppm_uart, float &co2_ppm_pwm, bool &presence_detected) {
    if (line == nullptr) {
        return false;
    }

    // Accepte les lignes de log qui prefixent la trame,
    // ex: "[UART_LINK] Trame envoyee: CO2_UART:0;CO2_PWM:0;PRES:0"
    const char *payload = strstr(line, "CO2_UART:");
    if (payload == nullptr) {
        return false;
    }

    float parsed_co2_uart = 0.0f;
    float parsed_co2_pwm = 0.0f;
    int parsed_presence = 0;

    // Format principal: CO2_UART:700;CO2_PWM:690;PRES:1
    if (sscanf(payload, "CO2_UART:%f;CO2_PWM:%f;PRES:%d", &parsed_co2_uart, &parsed_co2_pwm, &parsed_presence) == 3) {
        co2_ppm_uart = parsed_co2_uart;
        co2_ppm_pwm = parsed_co2_pwm;
        presence_detected = (parsed_presence != 0);
        return true;
    }

    return false;
}
}

// Initialisation des capteurs

void sensor_init() {
    dht.begin();
    // DHT22 a souvent besoin d'un court temps de stabilisation apres begin().
    delay(2000);
    printf("Capteur DHT initialise\n");
}

// Lire les capteurs et mettre à jour les données globales

bool sensor_read_dht(float &temperature, float &humidity) {
    static uint32_t invalid_count = 0;
    static bool dht_was_failing = false;

    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    if (!isnan(temperature) && !isnan(humidity)) {
        if (dht_was_failing) {
            printf("[DHT] Lecture revenue a la normale\n");
        }
        invalid_count = 0;
        dht_was_failing = false;
        return true;
    }

    invalid_count++;
    dht_was_failing = true;
    if (invalid_count == 3 || (invalid_count % 10) == 0) {
        printf("[DHT] Lecture invalide consecutive (%lu)\n", static_cast<unsigned long>(invalid_count));
    }
    return false;
}

void sensor_uart_init() {
    rs232.begin(RS232_BAUD, SERIAL_8N1, RS232_RX_PIN, RS232_TX_PIN);
    printf("RS232 activee (%d bauds) RX=%d TX=%d\n", RS232_BAUD, RS232_RX_PIN, RS232_TX_PIN);
}

bool sensor_uart_read_remote(float &co2_ppm_uart, float &co2_ppm_pwm, bool &presence_detected) {
    while (rs232.available() > 0) {
        const char c = static_cast<char>(rs232.read());

        if (c == '\r' || c == '\n') {
            if (rx_len == 0) {
                continue;
            }

            rx_line[rx_len] = '\0';
            const bool parsed = parse_remote_frame(rx_line, co2_ppm_uart, co2_ppm_pwm, presence_detected);
            if (!parsed) {
                printf("[RS232] Trame ignoree: %s\n", rx_line);
            } else {
                printf("[RS232] Trame OK U=%.0f P=%.0f PRES=%d\n", co2_ppm_uart, co2_ppm_pwm, presence_detected ? 1 : 0);
            }
            rx_len = 0;
            return parsed;
        }

        if (rx_len < (RX_BUFFER_SIZE - 1)) {
            rx_line[rx_len++] = c;
        } else {
            // Overflow: on abandonne la trame courante.
            rx_len = 0;
        }
    }

    return false;
}


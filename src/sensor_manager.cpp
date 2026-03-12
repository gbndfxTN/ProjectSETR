#include "sensor_manager.h"
#include "sensor_data.h"

// Instance du DHT
DHT dht(DHT_PIN, DHT_TYPE);

// Initialisation des capteurs

void sensor_init() {
    dht.begin();
    pinMode(GAS_PIN, INPUT);
    pinMode(ULTRASON_TRIG_PIN, OUTPUT);
    pinMode(ULTRASON_ECHO_PIN, INPUT);
    printf("Capteurs initialisés\n");
}

// Lire les capteurs et mettre à jour les données globales

void sensor_read_dht() {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    if (!isnan(temperature) && !isnan(humidity)) {
        sensor_data_set_dht(temperature, humidity);
    }
}

void sensor_read_gas() {
    float gas_resistance = analogRead(GAS_PIN);
    sensor_data_set_gas(gas_resistance);
}

void sensor_read_ultrason() {
    // Envoyer une impulsion de 10µs
    digitalWrite(ULTRASON_TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(ULTRASON_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRASON_TRIG_PIN, LOW);

    // Mesurer la durée de l'impulsion de retour
    long duration = pulseIn(ULTRASON_ECHO_PIN, HIGH);
    
    // Calculer la distance (vitesse du son = 343 m/s)
    float distance = (duration / 2.0) * 0.0343; // en cm
    sensor_data_set_ultrason(distance);
}

// Lire tous les capteurs
void sensor_read_all() {
    sensor_read_dht();
    sensor_read_gas();
    sensor_read_ultrason();
}


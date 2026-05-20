# ProjectSETR — ESP32 Central (Station de base)

Système embarqué sur ESP32 avec écran OLED, capteur DHT22, réception de données distantes via RS232, dashboard web et envoi vers Firebase Realtime Database.

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                   ProjectSETR (ESP32)                │
│  ┌──────────────┐  ┌─────────────┐  ┌────────────┐  │
│  │ DHT22 (GPIO15)│  │  UART2 RX   │  │  OLED SSD1306│ │
│  │  ─┐          │  │  (GPIO26)   │  │  (I2C)      │  │
│  │   ├──task▸Q   │  │   ─┐        │  │  ─┐        │  │
│  │  ─┘          │  │    ├──task▸Q  │  │   ├──task  │  │
│  │  Producer    │  │  ─┘        │  │  │ Consumer  │  │
│  └──────────────┘  └──┬──────────┘  └────────────┘  │
│                       │ Q                            │
│                 ┌─────▼──────┐  ┌──────────────────┐ │
│                 │ Firebase   │  │ Web Dashboard    │ │
│                 │ Uploader   │  │ (port 80, /api)  │ │
│                 └────────────┘  └──────────────────┘ │
└─────────────────────────────────────────────────────┘
                         ▲
                  RS232  │
┌─────────────────────────────────────────────────────┐
│                 ProjectSETR2 (ESP32 distant)          │
│  ┌──────────────┐  ┌─────────────┐  ┌────────────┐  │
│  │ MH-Z19B CO2  │  │ HC-SR04     │  │  UART2 TX  │  │
│  │ (UART+PWM)   │  │ Ultrason    │  │  (GPIO27)  │  │
│  └──────────────┘  └─────────────┘  └────────────┘  │
└─────────────────────────────────────────────────────┘
```

## Matériel

| Composant | Connexion |
|---|---|
| ESP32 DevKit V4 | Carte principale |
| DHT22 | GPIO15 |
| SSD1306 OLED 128×64 | I2C (SDA=GPIO5, SCL=GPIO4, addr=0x3C) |
| RS232 (liaison ESP32 distant) | UART2 (RX=GPIO26, TX=GPIO27, 9600 bauds) |

## Tâches FreeRTOS

| Tâche | Rôle | Pile | Cœur |
|---|---|---|---|
| `DhtProducer` | Lit DHT22 toutes les 5s, envoie dans la queue | 4096 | 1 |
| `UartProducer` | Reçoit trames RS232, envoie dans la queue | 4096 | 1 |
| `DisplayConsumer` | Consomme la queue, met à jour l'OLED à 30 FPS | 8192 | 1 |
| `FirebaseUploader` | Upload current + historique vers Firebase RTDB | 8192 | 0 |

## Protocole RS232

Trame reçue depuis ProjectSETR2 (format ASCII) :

```
CO2_UART:<ppm>;CO2_PWM:<ppm>;PRES:<0|1>\n
```

## Dépendances (PlatformIO)

- DHT sensor library (Adafruit)
- Adafruit Unified Sensor
- Adafruit SSD1306
- Adafruit GFX Library

## Construction et déploiement

```bash
pio run -t upload
pio device monitor
```

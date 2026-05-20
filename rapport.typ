#import "@preview/fletcher:0.5.7" as fletcher

#set page(margin: (x: 1.8cm, y: 1.5cm))
#set text(font: "Helvetica", size: 10pt)
#set par(leading: 0.35em)
#set heading(numbering: "1.1")
#set figure(numbering: "1")

#show heading.where(level: 1): it => {
  set text(size: 14pt, weight: "bold")
  it
  v(0.15cm)
}
#show heading.where(level: 2): it => {
  set text(size: 11pt, weight: "bold")
  it
  v(0.1cm)
}
#show heading.where(level: 3): it => {
  set text(size: 10pt, weight: "bold")
  it
  v(0.05cm)
}

#set outline(indent: auto)

// Page de garde
#align(center, text(size: 18pt, weight: "bold")[
  Projet SETR
])
#v(0.2cm)
#align(center, text(size: 10pt, style: "italic")[
  Analyse et modélisation selon la méthode SART
])

#v(0.4cm)
#outline()

#pagebreak()

= Introduction

Ce rapport présente l'analyse et la modélisation du système embarqué SETR distribué sur deux cartes ESP32 communiquant par liaison série UART. Il couvre la modélisation SART, l'architecture matérielle et logicielle, l'analyse temporelle ainsi que les solutions retenues et les résultats obtenus.

Fonctions assurées :
+ Mesure température/humidité (DHT22) et CO₂ par deux voies UART+PWM (MH-Z19B)
+ Détection de présence ultrason (HC-SR04)
+ Affichage OLED 128×64, dashboard web temps réel + API JSON
+ Upload périodique vers Firebase Realtime Database

= Modélisation SART

La méthode SART (Structured Analysis for Real-Time systems) distingue :
- *Le modèle environnemental* : diagramme de contexte et actigramme (DFD niveau 0)
- *Le modèle comportemental* : DFD de niveau 1 et spécification des processus

== Diagramme de Contexte

#figure(
  fletcher.diagram(
    node-stroke: 1pt,
    node-corner-radius: 2pt,
    spacing: (2.2cm, 1.3cm),
    fletcher.node((0,0), [
      *Système SETR*
      #text(size: 7pt)[ESP32 central + distant]
    ], name: "sys", shape: "rect", fill: luma(230), width: 2.8cm, height: 1.2cm),
    fletcher.node((-1.2, 1), [*DHT22*], name: "dht"),
    fletcher.node((-1.2, 0), [*MH-Z19B*], name: "mhz"),
    fletcher.node((-1.2, -1), [*HC-SR04*], name: "hcsr"),
    fletcher.node((1.2, 1), [*Utilisateur* #text(size: 7pt)[(Web)]], name: "usr"),
    fletcher.node((1.2, 0), [*Firebase* RTDB], name: "fb"),
    fletcher.node((1.2, -1), [*OLED* SSD1306], name: "oled"),
    fletcher.edge((-1.2, 1), (0,0), "->", text(size: 6pt)[T°, Hum]),
    fletcher.edge((-1.2, 0), (0,0), "->", text(size: 6pt)[CO2]),
    fletcher.edge((-1.2, -1), (0,0), "<->", text(size: 6pt)[TRIG/ECHO]),
    fletcher.edge((0,0), (1.2, 1), "<->", text(size: 6pt)[HTTP]),
    fletcher.edge((0,0), (1.2, 0), "->", text(size: 6pt)[JSON]),
    fletcher.edge((0,0), (1.2, -1), "->", text(size: 6pt)[I2C]),
  ),
  caption: [Diagramme de contexte du système SETR]
)

#figure(
  table(
    columns: (auto, 3cm, 3cm),
    stroke: 0.3pt,
    table.header([*Entité*], [*Flux entrant*], [*Flux sortant*]),
    [DHT22], [---], [Température, Humidité],
    [MH-Z19B], [---], [CO2 UART, CO2 PWM],
    [HC-SR04], [Impulsion TRIG], [Impulsion ECHO],
    [Utilisateur (Web)], [Requêtes HTTP], [Page HTML, JSON API],
    [Firebase RTDB], [---], [Données PATCH/PUT JSON],
    [OLED SSD1306], [---], [Données I2C (affichage)],
  ),
  caption: [Flux échangés avec l'environnement]
)

== Actigramme (DFD Niveau 0)

Le système est décomposé en quatre processus. Les capteurs distants (MH-Z19B, HC-SR04) sont acquis par l'ESP32 déporté puis transmis à la carte centrale via RS232 (processus 2, flux interne).

#figure(
  fletcher.diagram(
    node-stroke: 1pt,
    node-corner-radius: 2pt,
    spacing: (1.4cm, 0.85cm),
    fletcher.node((-2.5, 1.5), [*DHT22*], name: "dht0", width: 1.4cm),
    fletcher.node((-2.5, 0), [*Capteurs* #text(size: 7pt)[distants]], name: "dist0", width: 1.4cm),
    fletcher.node((-2.5, -1.5), [*Utilisateur*], name: "u0", width: 1.4cm),
    fletcher.node((0, 1.5), [#text(size: 6pt)[1. Acq. loc.]], name: "p1", shape: "circle", fill: luma(240), width: 1.3cm, height: 1.1cm),
    fletcher.node((0, 0), [#text(size: 6pt)[2. Récept. RS232]], name: "p2", shape: "circle", fill: luma(240), width: 1.3cm, height: 1.1cm),
    fletcher.node((2, 1), [#text(size: 6pt)[3. Affichage]], name: "p3", shape: "circle", fill: luma(240), width: 1.5cm, height: 1.3cm),
    fletcher.node((2, -1), [#text(size: 6pt)[4. Diffusion]], name: "p4", shape: "circle", fill: luma(240), width: 1.5cm, height: 1.3cm),
    fletcher.node((4, 1.5), [#text(size: 7pt)[D1 État Capteurs]], name: "d1", shape: "rect", stroke: (dash: "dashed"), fill: luma(250), width: 1.4cm, height: 0.9cm),
    fletcher.node((4, -0.5), [#text(size: 7pt)[D2 Affichage]], name: "d2", shape: "rect", stroke: (dash: "dashed"), fill: luma(250), width: 1.4cm, height: 0.9cm),
    fletcher.node((4, -2.2), [*Firebase*], name: "fb0", width: 1.4cm),
    fletcher.node((2, -2.8), [*Web*], name: "web0", width: 1.2cm),
    fletcher.node((0, -2.8), [*OLED*], name: "oled0", width: 1.2cm),
    fletcher.edge((-2.5, 1.5), (0, 1.5), "->"),
    fletcher.edge((0, 1.5), (4, 1.5), "->"),
    fletcher.edge((-2.5, 0), (0, 0), "->"),
    fletcher.edge((0, 0), (4, 1.5), "->"),
    fletcher.edge((4, 1.5), (2, 1), "->"),
    fletcher.edge((2, 1), (4, -0.5), "->"),
    fletcher.edge((4, -0.5), (2, -1), "->"),
    fletcher.edge((2, 1), (0, -2.8), "->"),
    fletcher.edge((2, -1), (4, -2.2), "->"),
    fletcher.edge((2, -1), (2, -2.8), "->"),
    fletcher.edge((-2.5, -1.5), (2, -2.8), "<->", text(size: 6pt)[HTTP]),
  ),
  caption: [Actigramme (DFD niveau 0)]
)

== Dictionnaire des Données

#figure(
  table(
    columns: (auto, 1fr, auto),
    stroke: 0.3pt,
    table.header([*Flux*], [*Description*], [*Type*]),
    [Température], [Mesure ambiante], [float °C],
    [Humidité], [Humidité relative], [float %],
    [CO2_UART], [Concentration CO2 via UART], [int ppm],
    [CO2_PWM], [Concentration CO2 via PWM], [int ppm],
    [Présence], [Détection ultrason], [bool],
    [Trame RS232], [Trame ASCII "CO2_UART:...;CO2_PWM:...;PRES:..."], [string],
    [Page Web], [Dashboard HTML responsive], [string],
    [API JSON], [Données temps réel], [string],
    [Données Firebase], [Payload JSON vers RTDB], [string],
  ),
  caption: [Dictionnaire des flux de données]
)

#figure(
  table(
    columns: (auto, 1fr, auto),
    stroke: 0.3pt,
    table.header([*Stock*], [*Contenu*], [*Structure*]),
    [État Capteurs (D1)], [Dernières valeurs des capteurs], [`SensorData`],
    [Données Affichage (D2)], [Lignes de texte pour OLED], [`DisplayData`],
  ),
  caption: [Stocks de données]
)

#figure(
  ```cpp
  struct SensorData {
    float temperature;         // °C
    float humidity;            // %
    float co2_ppm_uart;        // ppm
    float co2_ppm_pwm;         // ppm
    bool  presence_detected;   // true/false
    unsigned long last_update_dht;
    unsigned long last_update_remote;
  };
  struct SensorMessage {
    SensorMessageType type;    // Dht ou Remote
    float value1, value2;      // données capteur
    bool  flag;                // présence
  };
  ```,
  caption: [Structures de données]
)

== DFD Niveau 1 — Carte Centrale (ProjectSETR)

Quatre tâches FreeRTOS interconnectées par une file de messages `SensorQueue` (16 éléments).

#figure(
  fletcher.diagram(
    node-stroke: 1pt,
    node-corner-radius: 2pt,
    spacing: (1.3cm, 1.0cm),
    fletcher.node((-2.5, 1.5), [*DHT22*], name: "dht1", width: 1.3cm),
    fletcher.node((-2.5, 0), [*RS232*], name: "rs1", width: 1.3cm),
    fletcher.node((-0.5, 1.5), [#text(size: 9pt)[*DHT* #text(size: 7pt)[(producteur)]]], name: "dp", shape: "rect", fill: luma(240), width: 1.8cm, height: 0.9cm),
    fletcher.node((-0.5, 0), [#text(size: 9pt)[*UART* #text(size: 7pt)[(producteur)]]], name: "up", shape: "rect", fill: luma(240), width: 1.8cm, height: 0.9cm),
    fletcher.node((1.5, 0.75), [#text(size: 8pt)[*Queue* #text(size: 6pt)[×16]]], name: "sq", shape: "rect", stroke: (dash: "dashed"), fill: luma(250), width: 1.4cm, height: 0.8cm),
    fletcher.node((3.5, 1.5), [#text(size: 9pt)[*Affichage* #text(size: 7pt)[(consommateur)]]], name: "dc", shape: "rect", fill: luma(220), width: 2.2cm, height: 1.0cm),
    fletcher.node((3.5, 0), [#text(size: 9pt)[*Firebase* #text(size: 7pt)[(consommateur)]]], name: "fu", shape: "rect", fill: luma(240), width: 2.2cm, height: 1.0cm),
    fletcher.node((3.5, -1.2), [#text(size: 9pt)[*Web* #text(size: 7pt)[(consommateur)]]], name: "wd", shape: "rect", fill: luma(240), width: 1.8cm, height: 0.9cm),
    fletcher.node((5.5, 1.5), [*OLED*], name: "oled1", width: 1.3cm),
    fletcher.node((5.5, 0), [*Firebase*], name: "fb1", width: 1.3cm),
    fletcher.node((5.5, -1.2), [*Navigateur*], name: "nav1", width: 1.3cm),
    fletcher.edge((-2.5, 1.5), (-0.5, 1.5), "->"),
    fletcher.edge((-2.5, 0), (-0.5, 0), "->"),
    fletcher.edge((-0.5, 1.5), (1.5, 0.75), "->"),
    fletcher.edge((-0.5, 0), (1.5, 0.75), "->"),
    fletcher.edge((1.5, 0.75), (3.5, 1.5), "->"),
    fletcher.edge((1.5, 0.75), (3.5, 0), "->"),
    fletcher.edge((1.5, 0.75), (3.5, -1.2), "->"),
    fletcher.edge((3.5, 1.5), (5.5, 1.5), "->"),
    fletcher.edge((3.5, 0), (5.5, 0), "->"),
    fletcher.edge((3.5, -1.2), (5.5, -1.2), "<->", text(size: 6pt)[HTTP]),
  ),
  caption: [DFD niveau 1 : architecture multitâche de la carte centrale]
)

== DFD Niveau 1 — Carte Distante (ProjectSETR2)

Trois tâches FreeRTOS en pattern producteur/consommateur. Les mesures CO2 (UART+PWM) et présence sont produites périodiquement, consommées par la tâche `uart_consumer` qui formate et émet la trame sur UART2.

#figure(
  fletcher.diagram(
    node-stroke: 1pt,
    node-corner-radius: 2pt,
    spacing: (1.4cm, 1.2cm),
    fletcher.node((-2, 1.5), [*MH-Z19B*], name: "mhz1", width: 1.5cm),
    fletcher.node((-2, -0.5), [*HC-SR04*], name: "hcsr1", width: 1.5cm),
    fletcher.node((0, 1.5), [#text(size: 9pt)[*CO2* #text(size: 7pt)[(producteur)]]], name: "c2p", shape: "rect", fill: luma(240), width: 1.8cm, height: 0.9cm),
    fletcher.node((0, -0.5), [#text(size: 9pt)[*Présence* #text(size: 7pt)[(producteur)]]], name: "prp", shape: "rect", fill: luma(240), width: 1.8cm, height: 0.9cm),
    fletcher.node((2.5, 0.5), [#text(size: 8pt)[*Queue* #text(size: 6pt)[×16]]], name: "sq2", shape: "rect", stroke: (dash: "dashed"), fill: luma(250), width: 1.3cm, height: 0.8cm),
    fletcher.node((4.5, 0.5), [#text(size: 9pt)[*UART TX* #text(size: 7pt)[(consommateur)]]], name: "uc", shape: "rect", fill: luma(220), width: 2.0cm, height: 0.9cm),
    fletcher.node((6, 0.5), [*UART2* #text(size: 6pt)[vers central]], name: "uart2", width: 1.3cm),
    fletcher.edge((-2, 1.5), (0, 1.5), "->"),
    fletcher.edge((-2, -0.5), (0, -0.5), "->"),
    fletcher.edge((0, 1.5), (2.5, 0.5), "->"),
    fletcher.edge((0, -0.5), (2.5, 0.5), "->"),
    fletcher.edge((2.5, 0.5), (4.5, 0.5), "->"),
    fletcher.edge((4.5, 0.5), (6, 0.5), "->"),
  ),
  caption: [DFD niveau 1 : architecture multitâche de la carte distante]
)

= Spécification des Tâches

#figure(
  table(
    columns: (auto, auto, auto, auto, auto, auto),
    stroke: 0.3pt,
    table.header([*Tâche*], [*Carte*], [*Pile*], [*Cœur*], [*Prio.*], [*Période*]),
    [DhtProducer], [Centrale], [4096], [1], [1], [5 s],
    [UartProducer], [Centrale], [4096], [1], [1], [10 ms (poll)],
    [DisplayConsumer], [Centrale], [8192], [1], [2], [33 ms],
    [FirebaseUploader], [Centrale], [8192], [0], [1], [5 s / 60 s],
    [WebDashboard], [Centrale], [8192], [0], [1], [Apériodique],
    [co2_producer], [Distante], [3072], [---], [5], [4 s],
    [presence_producer], [Distante], [4096], [---], [5], [500 ms],
    [uart_consumer], [Distante], [4096], [---], [6], [4 s],
  ),
  caption: [Spécification des tâches FreeRTOS]
)

*Périodiques* : DhtProducer, co2_producer, presence_producer. *Quasi-périodiques* : DisplayConsumer, FirebaseUploader, uart_consumer (activés par file/timer). *Apériodique* : WebDashboard (requêtes HTTP). *Polling* : UartProducer (scrutation buffer UART).

= Architecture Matérielle

#grid(columns: (1fr, 1fr), gutter: 1cm,
  figure(
    table(
      columns: (auto, auto, auto),
      stroke: 0.3pt,
      table.header([*Composant*], [*Broche*], [*Protocole*]),
      [DHT22], [GPIO15], [OneWire],
      [SSD1306 OLED], [SDA=GPIO5, SCL=GPIO4], [I2C (0x3C)],
      [RS232 RX/TX], [GPIO26/27 (UART2)], [UART 9600 8N1],
    ),
    caption: [Carte Centrale (ProjectSETR)]
  ),
  figure(
    table(
      columns: (auto, auto, auto),
      stroke: 0.3pt,
      table.header([*Composant*], [*Broche*], [*Protocole*]),
      [MH-Z19B UART], [TX=GPIO16, RX=GPIO17], [UART1 9600 8N1],
      [MH-Z19B PWM], [GPIO23], [PWM (5000 ppm)],
      [HC-SR04], [TRIG=GPIO33, ECHO=GPIO32], [GPIO out/in],
      [RS232 TX/RX], [GPIO27/26 (UART2)], [UART 9600 8N1],
    ),
    caption: [Carte Distante (ProjectSETR2)]
  )
)

= Architecture Logicielle

== Frameworks et arborescence

La carte centrale utilise le *framework Arduino* (bibliothèques Adafruit DHT22, SSD1306, GFX). La carte distante utilise le *framework ESP-IDF* (C bare-metal) pour un contrôle fin des GPIO/UART et une empreinte mémoire réduite. Les deux projets sont gérés sous PlatformIO.

#grid(columns: (1fr, 1fr), gutter: 1cm,
  ```text
  ProjectSETR/ (Arduino)
  src/
    main.cpp              - Setup + tâches
    sensor_manager.cpp    - DHT22 + parsing RS232
    sensor_data.cpp       - Accès thread-safe
    display_manager.cpp   - Pilotage OLED
    display_data.cpp      - Formatage affichage
    global_data.cpp       - Mutex + instances
    firebase_uploader.cpp - Upload WiFi
    web_dashboard.cpp     - Serveur HTTP + API
  include/                - Headers + config.h
  platformio.ini
  ```,
  ```text
  ProjectSETR2/ (ESP-IDF)
  src/
    main.c                - Init + tâches
    co2_sensor.c          - MH-Z19B (UART+PWM)
    presence_sensor.c     - HC-SR04
    uart_link.c           - Émission trame UART2
    consumer_task.c       - Queue -> UART TX
  include/                - Headers + config.h
  platformio.ini
  ```
)

== Protocole Inter-cartes

Communication UART à 9600 bauds. Format de trame ASCII :

```text
CO2_UART:<ppm>;CO2_PWM:<ppm>;PRES:<0|1>\n
```

Le parser de la carte centrale (`sensor_manager.cpp`) tolère un préfixe de log avant `CO2_UART:`, ce qui permet le débogage sur la même ligne série.

== Gestion de l'affichage OLED

L'OLED s'éteint automatiquement en l'absence de présence (économie d'énergie). Un pin de synchro externe (GPIO25, macro `USE_EXTERNAL_SYNC`) permet un pilotage depuis la carte distante. Sans données RS232, l'écran affiche un message d'attente avec les données locales uniquement.

Un mode simulation (`SIMULATE_REMOTE_ESP`) génère des données CO2/présence factices pour tester la chaîne complète sans la carte distante physique.

== Protocole Firebase

Données envoyées au Firebase Realtime Database via HTTP direct (sans le SDK Firebase, plus léger de ~500 Ko) :

+ *Courant* : HTTP PATCH toutes les 5 secondes
+ *Historique* : HTTP PUT toutes les 60 secondes (tampon tournant de 60 slots)

La reconnexion WiFi est automatique (intervalle de 5 s entre tentatives).

= Analyse Temporelle

== Contraintes et ordonnancement

#figure(
  table(
    columns: (1fr, auto, auto),
    stroke: 0.3pt,
    table.header([*Contrainte*], [*Valeur*], [*Critique*]),
    [Échantillonnage température/humidité], [5000 ms], [Non],
    [Échantillonnage CO2 (UART+PWM)], [4000 ms], [Non],
    [Échantillonnage présence], [500 ms], [Oui],
    [Rafraîchissement OLED], [33 ms (30 FPS)], [Non],
    [Envoi Firebase courant], [5000 ms], [Non],
    [Envoi Firebase historique], [60000 ms], [Non],
    [Réponse serveur web], [< 100 ms], [Non],
    [File de messages], [16 éléments], [---],
  ),
  caption: [Contraintes temps réel]
)

Tâches ordonnancées par FreeRTOS en préemptif à priorité fixe :
+ Affichage (priorité 2) servi avant les producteurs (priorité 1)
+ Sur la carte distante, `uart_consumer` (priorité 6) vidange la file avant les producteurs (priorité 5)
+ L'upload Firebase est isolé sur le cœur 0 pour ne pas perturber les tâches temps réel du cœur 1

== Gestion des accès concurrents

Accès aux données partagées protégés par *mutex FreeRTOS* : `sensors_mutex` (SensorData) et `display_mutex` (lignes OLED). Le pattern producteur/consommateur via *file de messages* (16 éléments) découple les tâches d'acquisition du traitement, évitant tout blocage direct.

= Solutions Retenues

*Architecture 2 ESP32* : un seul ESP32 ne dispose pas d'assez d'UART pour le MH-Z19B (UART1), le terminal (UART0) et la liaison inter-cartes. La distribution permet aussi de placer les capteurs dans une zone distincte de l'affichage.

*Liaison UART ASCII* : native sur ESP32, sans bibliothèque externe. 9600 bauds suffisent pour ~40 octets toutes les 4 secondes. Le format ASCII facilite le débogage.

*Pattern producteur/consommateur* : la `Queue` FreeRTOS de 16 éléments découple temporellement les acquisitions périodiques du traitement. Capacité équivalente à ~80 s de mesures CO2.

*Double voie CO2* : lecture simultanée UART + PWM du MH-Z19B pour redondance et fiabilité accrue.

*Frameworks* : Arduino pour la centrale (bibliothèques Adafruit), ESP-IDF pour la distante (contrôle bas niveau, empreinte réduite).

*HTTP direct Firebase* : plus léger que le SDK Firebase (~500 Ko). Suffisant pour des données RTDB publiques.

*Mode simulation intégré* : `SIMULATE_REMOTE_ESP` accélère les itérations de développement sans matériel distant.

= Résultats Obtenus

Toutes les fonctions du cahier des charges sont implémentées et testées :
+ DHT22 : température/humidité toutes les 5 s, tolérance aux erreurs (`isnan`)
+ MH-Z19B : CO2 UART+PWM toutes les 4 s, calibration zero et ABC configurables
+ HC-SR04 : distance toutes les 500 ms, seuil de présence à 50 cm
+ OLED : 4 lignes (T°, H%, CO2 PWM, CO2 UART), extinction automatique, mode attente
+ Dashboard web : page responsive, 5 indicateurs avec âge, rafraîchissement auto 3 s, API JSON `/api/data`
+ Firebase : upload courant (5 s) et historique (60 s), reconnexion WiFi automatique

Les contraintes temps réel sont respectées : la scrutation de présence (500 ms) est garantie, l'affichage 30 FPS est découplé, et l'upload Firebase (cœur 0) ne perturbe pas les tâches critiques.

*Limitations* : lectures DHT22 occasionnellement invalides (~2-3%), upload Firebase non authentifié (données publiques uniquement), test RS232 de bout en bout nécessite le matériel physique (Wokwi ne simule pas la liaison inter-ESP32).

= Conclusion

Le système SETR met en œuvre une architecture temps réel distribuée sur deux ESP32. La modélisation SART a formalisé les interactions capteurs/sorties (modèle environnemental), la décomposition multitâche FreeRTOS (modèle comportemental) et les structures de données partagées. L'architecture producteur/consommateur avec file de messages et mutex assure le découplage temporel et l'intégrité des données. Les contraintes critiques (présence à 500 ms, trames RS232 toutes les 4 s) sont respectées.

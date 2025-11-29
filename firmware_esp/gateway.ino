/*
 * ZAŘÍZENÍ 3: GATEWAY (ESP32-S3 Doma)
 * Funkce: Příjem LoRa -> WiFi -> MQTT Publish
 */
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <PubSubClient.h>

// --- PINY PRO LoRa (OPĚT UPRAV DLE DESKY) ---
#define SS_PIN   5
#define RST_PIN  14
#define DIO0_PIN 2
#define BAND 868E6

// --- WIFI A MQTT (Zkopírováno z tvého Pythonu) ---
const char* ssid = "TVOJE_WIFI_JMENO";        // <--- DOPLŇ
const char* password = "TVOJE_WIFI_HESLO";    // <--- DOPLŇ

const char* mqtt_server = "public.cloud.shiftr.io"; // Nebo adresa vašeho brokera
const char* mqtt_user = "VASE_UZIVATELSKE_JMENO";   // Např. "mujsklenik"
const char* mqtt_pass = "VASE_HESLO_NEBO_TOKEN";    // Token ze shiftr.io
const char* mqtt_topic = "sensor/sklenik/teplota";  // Téma, kam se data posílají

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Připojuji k WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi připojeno!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Připojuji k MQTT... ");
    String clientId = "ESP32Gateway-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("Připojeno!");
    } else {
      Serial.print("Chyba, rc=");
      Serial.print(client.state());
      Serial.println(" zkusím za 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // 1. Start LoRa
  SPI.begin();
  LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);
  if (!LoRa.begin(BAND)) {
    Serial.println("❌ LoRa selhala!");
    while (1);
  }
  Serial.println("✅ LoRa běží a poslouchá...");

  // 2. Start WiFi & MQTT
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  // Kontrola WiFi/MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // --- KONTROLA LORA PAKETŮ ---
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Přišel LoRa paket: ");
    String receivedText = "";
    
    while (LoRa.available()) {
      receivedText += (char)LoRa.read();
    }
    Serial.println(receivedText);

    // Odeslání na MQTT (tvůj Python čeká čisté číslo, přesně to posíláme)
    client.publish(mqtt_topic, receivedText.c_str());
    Serial.println("DATA ODESLÁNA NA CLOUD ☁️");
  }
}
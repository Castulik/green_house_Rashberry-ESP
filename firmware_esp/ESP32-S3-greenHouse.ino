/*
 * ZAŘÍZENÍ 2: HUB/BRIDGE (ESP32-S3 ve skleníku)
 * Funkce: Přijme ESP-NOW -> Odešle LoRa
 */
#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <LoRa.h>

// --- PINY PRO LoRa (UPRAV DLE SVÉ DESKY!) ---
// Pro ESP32-S3 Heltec v3 jsou často: SS=8, RST=12, DIO0=14
// Pro Generic ESP32 + modul: SS=5, RST=14, DIO0=2
#define SS_PIN   5
#define RST_PIN  14
#define DIO0_PIN 2

// Frekvence: 433E6, 868E6 nebo 915E6 (dle tvého modulu)
#define BAND 868E6 

// Struktura příchozích dat
typedef struct struct_message {
  float temp;
  float hum;
} struct_message;

struct_message myData;

// Callback: Když přijdou data přes ESP-NOW
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Příjem ESP-NOW: ");
  Serial.print(myData.temp);
  Serial.println(" C");

  // --- ODESLÁNÍ PŘES LORA ---
  Serial.print("Přeposílám přes LoRa... ");
  
  LoRa.beginPacket();
  // Posíláme jen teplotu jako text (aby to bylo jednoduché pro bránu)
  // Můžeš poslat i: String(myData.temp) + ";" + String(myData.hum)
  LoRa.print(myData.temp); 
  LoRa.endPacket();
  
  Serial.println("Hotovo.");
}

void setup() {
  Serial.begin(115200);

  // 1. Start LoRa
  SPI.begin(); // Případně SPI.begin(sck, miso, mosi, ss);
  LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);
  
  if (!LoRa.begin(BAND)) {
    Serial.println("❌ LoRa selhala! Zkontroluj piny.");
    while (1);
  }
  Serial.println("✅ LoRa OK");

  // 2. Start ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Chyba ESP-NOW");
    return;
  }
  
  // Registrace funkce pro příjem
  esp_now_register_recv_cb(OnDataRecv); // Pro ESP32 verze 2.x a nižší
  // Pokud používáš novou verzi Arduino ESP32 (3.0+), může to chtít jinou signaturu:
  // esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

void loop() {
  // Hub jen čeká na přerušení od ESP-NOW, loop je prázdný
  delay(1000);
}
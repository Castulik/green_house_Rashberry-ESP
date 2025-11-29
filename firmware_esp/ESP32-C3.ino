/*
 * ZAŘÍZENÍ 1: SENZOR (ESP32-C3)
 * Funkce: Změří teplotu -> Pošle přes ESP-NOW -> Deep Sleep
 */
#include <esp_now.h>
#include <WiFi.h>
#include "DHT.h"

// --- NASTAVENÍ ---
#define DHTPIN 4       // Pin, kde máš připojený senzor (uprav dle reality)
#define DHTTYPE DHT22  // nebo DHT11
#define SLEEP_SEC 600  // Jak dlouho spát (v sekundách) - 10 minut

DHT dht(DHTPIN, DHTTYPE);

// Adresa pro broadcast (pošle to všem v okolí)
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Struktura dat (musí být stejná na vysílači i přijímači)
typedef struct struct_message {
  float temp;
  float hum;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

// Callback po odeslání (jen pro kontrolu)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Stav odeslání: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Úspěch" : "Chyba");
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Zapneme WiFi v režimu Station
  WiFi.mode(WIFI_STA);

  // Inicializace ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Chyba inicializace ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  // Registrace Peer (Broadcast)
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Chyba přidání peer");
    return;
  }

  // --- MĚŘENÍ ---
  // Čtení teploty (pokud nemáš senzor, odkomentuj řádek s random)
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  // Pokud senzor selže, pošleme nulu nebo chybovou hodnotu
  if (isnan(t)) t = 0.0;
  if (isnan(h)) h = 0.0;
  
  // Pro testování bez senzoru odkomentuj:
  // t = 20.0 + (random(0, 100) / 10.0); 

  myData.temp = t;
  myData.hum = h;

  // --- ODESLÁNÍ ---
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Odesláno: " + String(t) + "C");
  } else {
    Serial.println("Chyba odeslání");
  }

  delay(100); // Krátká pauza, aby se stihla data odeslat

  // --- SPÁNEK ---
  Serial.println("Jdu spát...");
  esp_deep_sleep_start();
  // Zbytek kódu se nikdy neprovede, po probuzení začíná setup() znova
}

void loop() {
  // Zde nic není, vše řeší setup
}
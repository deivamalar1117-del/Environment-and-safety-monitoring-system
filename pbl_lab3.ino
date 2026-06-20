#include <WiFi.h>
#include <DHT.h>
#include <ThingSpeak.h>
#include "config.h" // This line pulls in your hidden passwords!

// --- Wi-Fi Credentials & ThingSpeak Setup ---
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
unsigned long myChannelNumber = TS_CHANNEL_NUMBER; 
const char * myWriteAPIKey = TS_WRITE_API_KEY; 
WiFiClient client;

// --- Pin Configurations ---
#define DHTPIN 4
#define DHTTYPE DHT22
#define SOS_BUTTON_PIN 15

DHT dht(DHTPIN, DHTTYPE);

// --- Timer Variables ---
unsigned long lastDHTRead = 0;
const unsigned long dhtInterval = 20000; 

unsigned long lastWiFiCheck = 0;
const unsigned long wiFiCheckInterval = 10000; 

unsigned long lastThingSpeakWrite = 0;

// --- Debouncing Variables for SOS Button ---
volatile bool sosTriggered = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 250; 

// --- Interrupt Service Routine (ISR) ---
void IRAM_ATTR handleSOSInterrupt() {
  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTime) > debounceDelay) {
    sosTriggered = true;
    lastDebounceTime = currentTime;
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  pinMode(SOS_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SOS_BUTTON_PIN), handleSOSInterrupt, FALLING);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  
  ThingSpeak.begin(client);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastWiFiCheck >= wiFiCheckInterval) {
    lastWiFiCheck = currentMillis;
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("[Warning] Wi-Fi lost. Attempting background reconnect...");
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    }
  }

  if (sosTriggered) {
    sosTriggered = false;
    Serial.println("[ALERT] SOS Button Pressed! Sending to ThingSpeak...");
    sendToThingSpeak(dht.readTemperature(), dht.readHumidity(), true);
  }

  if (currentMillis - lastDHTRead >= dhtInterval) {
    lastDHTRead = currentMillis;
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    if (!isnan(temp) && !isnan(hum)) {
      Serial.println("[Data] Sending normal telemetry to ThingSpeak.");
      sendToThingSpeak(temp, hum, false);
    } else {
      Serial.println("[Error] Failed to read from DHT22 sensor.");
    }
  }
}

void sendToThingSpeak(float temp, float humidity, bool isEmergency) {
  if (WiFi.status() == WL_CONNECTED) {
    
    unsigned long timeSinceLastWrite = millis() - lastThingSpeakWrite;
    if (lastThingSpeakWrite != 0 && timeSinceLastWrite < 15500) {
      unsigned long waitTime = 15500 - timeSinceLastWrite;
      Serial.print("[ThingSpeak] Rate limit active. Holding payload for ");
      Serial.print(waitTime / 1000);
      Serial.println(" seconds...");
      delay(waitTime);
    }

    ThingSpeak.setField(1, temp);
    ThingSpeak.setField(2, humidity);
    ThingSpeak.setField(3, isEmergency ? 1 : 0);
    
    int tsResponseCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if(tsResponseCode == 200){
      Serial.println("[ThingSpeak] Channel update successful.");
      lastThingSpeakWrite = millis();
    } else {
      Serial.println("[ThingSpeak] Problem updating channel. HTTP error code " + String(tsResponseCode));
    }
    
  } else {
    Serial.println("[ThingSpeak] Transmit failed: Wi-Fi network currently unavailable.");
  }
}
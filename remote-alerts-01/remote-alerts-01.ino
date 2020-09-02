#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>
#include <WiFiClient.h>

const char *ssid = "*****";      // Enter your WIFI ssid
const char *password = "*****";  // Enter your WIFI password
ESP8266WebServer server(8700);

// An IR detector/demodulator is connected to GPIO pin 14(D5 on a NodeMCU
// board). Note: GPIO 16 won't work on the ESP8266 as it does not have
// interrupts.
const uint16_t kRecvPin = 14;
const int LED = 15;
const int BUZZER = 5;
const int TONE = 2000;  // 2Khz.
int enabled = 0;

IRrecv irrecv(kRecvPin);

decode_results results;

void sendStatus() {
  if (enabled == 1) {
    server.send(200, "application/json", "{\"enabled\": true}");
  } else {
    server.send(200, "application/json", "{\"enabled\": false}");
  }
}

void onEnable() {
  Serial.println("[ WEB ]-> ENABLED");
  enabled = 1;
  sendStatus();
}

void onDisable() {
  Serial.println("[ WEB ]-> DISABLED");
  enabled = 0;
  sendStatus();
}

void route() {
  server.on("/", senStatus;
  server.on("/enable", onEnable);
  server.on("/disable", onDisable);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  delay(3000);

  Serial.print("Configuring access point...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  route();

  pinMode(LED, OUTPUT);
  irrecv.enableIRIn();  // Start the receiver
  Serial.println();
  Serial.println("Started! Waiting for IR signal...");
}

void blinkBuzz(int blinks, int blinkDelay = 100) {
  for (int i = 0; i < blinks; i++) {
    digitalWrite(LED, HIGH);
    tone(BUZZER, TONE, blinkDelay);
    delay(blinkDelay);

    digitalWrite(LED, LOW);
    tone(BUZZER, TONE * 1.5, blinkDelay);
    delay(blinkDelay);
  }
}

void blink(int blinks, int blinkDelay = 100) {
  for (int i = 0; i < blinks; i++) {
    digitalWrite(LED, HIGH);
    delay(blinkDelay);

    digitalWrite(LED, LOW);
    delay(blinkDelay);
  }
}

void loop() {
  if (enabled == 1) {
    blinkBuzz(3);
  }

  if (irrecv.decode(&results)) {
    serialPrintUint64(results.value, HEX);
    Serial.println("");

    if (results.value == 0xFFC23D) {
      Serial.println("[ IR ]-> DISABLED");
      blink(3);
      enabled = 0;
    }

    irrecv.resume();  // Receive the next value
  }
  delay(100);
}

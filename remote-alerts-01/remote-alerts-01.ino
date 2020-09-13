#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>
#include <WiFiClient.h>

ESP8266WebServer server(80);

// WiFi config.
const char *ssid = "";
const char *password = "";
IPAddress staticIP(192, 168, 1, 246);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);
const char *deviceName = "berthing-alarm";

// An IR detector/demodulator is connected to GPIO pin 14(D5 on a NodeMCU
// board). Note: GPIO 16 won't work on the ESP8266 as it does not have
// interrupts.
const uint16_t kRecvPin = 14;
const int LED = 15;
const int BUZZER = 5;
unsigned long last = millis();
int blinksCount = 0;
int ledState = 0;
int enabled = 0;
int silent = 0;

IRrecv irrecv(kRecvPin);
decode_results results;

void sendHello() { server.send(200, "text/plain", "Hello from ESP8266!\r\n"); }

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
  silent = 0;
  sendStatus();
}

void onEnableSilent() {
  Serial.println("[ WEB ]-> ENABLED");
  enabled = 1;
  silent = 1;
  sendStatus();
}

void onDisable() {
  Serial.println("[ WEB ]-> DISABLED");
  enabled = 0;
  sendStatus();
}

void route() {
  server.on("/", sendStatus);
  server.on("/hello", sendHello);
  server.on("/enable", onEnable);
  server.on("/silent", onEnableSilent);
  server.on("/disable", onDisable);
}

void blinkBuzzDelay(int blinks, int blinkDelay = 100, int freq = 2000) {
  for (int i = 0; i < blinks; i++) {
    digitalWrite(LED, HIGH);
    tone(BUZZER, freq, blinkDelay);
    delay(blinkDelay);

    digitalWrite(LED, LOW);
    tone(BUZZER, freq * 1.5, blinkDelay);
    delay(blinkDelay);
  }
}

void blinkDelay(int blinks = 1, int blinkDelay = 100) {
  for (int i = 0; i < blinks; i++) {
    digitalWrite(LED, HIGH);
    delay(blinkDelay);

    digitalWrite(LED, LOW);
    delay(blinkDelay);
  }
}

void blinkBuzz(long interval = 100, int blinks = 3, long pause = 500,
               int freq = 2000) {
  unsigned long current = millis();
  if (current - last < interval) return;
  if (blinksCount == blinks && current - last < pause) return;
  last = current;

  if (ledState == 0) {
    // Turn on.
    digitalWrite(LED, HIGH);
    tone(BUZZER, freq, 100);
    ledState = 1;
    if (blinksCount == blinks) blinksCount = 0;
  } else {
    // Turn off.
    digitalWrite(LED, LOW);
    tone(BUZZER, freq * 1.5, 100);
    ledState = 0;
    blinksCount = blinksCount + 1;
  }
}

void blink(long interval = 100, int blinks = 3, long pause = 500) {
  unsigned long current = millis();
  if (current - last < interval) return;
  if (blinksCount == blinks && current - last < pause) return;
  last = current;

  if (ledState == 0) {
    // Turn on.
    digitalWrite(LED, HIGH);
    ledState = 1;
    if (blinksCount == blinks) blinksCount = 0;
  } else {
    // Turn off.
    digitalWrite(LED, LOW);
    ledState = 0;
    blinksCount = blinksCount + 1;
  }
}

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  blinkDelay();

  Serial.begin(115200);
  delay(3000);
  Serial.print("Configuring access point...");

  WiFi.hostname(deviceName);
  WiFi.config(staticIP, subnet, gateway, dns);
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

  irrecv.enableIRIn();  // Start the receiver
  Serial.println();
  Serial.println("Started! Waiting for IR signal...");
  blinkDelay(2);
}

void loop() {
  // Answer web server requests.
  server.handleClient();

  if (enabled == 1) {
    if (silent == 0) {
      blinkBuzz();
    } else {
      blink();
    }
  }

  if (irrecv.decode(&results)) {
    // if (irrecv.decode(&results) && results.decode_type == 12) {
    // serialPrintUint64(results.value, HEX);
    // Serial.println("");

    // Disable alarm.
    if (results.value == 0xE240) {
      Serial.println("[ IR ]-> DISABLED");
      blinkDelay(2);
      enabled = 0;
      silent = 0;
    }

    // Enable noisy alarm.
    if (results.value == 0xE250) {
      Serial.println("[ IR ]-> ENABLED");
      enabled = 1;
      silent = 0;
    }

    // Enable silent alarm.
    if (results.value == 0xE248) {
      Serial.println("[ IR ]-> ENABLED (silent)");
      enabled = 1;
      silent = 1;
    }

    irrecv.resume();  // Receive the next value.
  }
}

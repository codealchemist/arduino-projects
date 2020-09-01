#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

// An IR detector/demodulator is connected to GPIO pin 14(D5 on a NodeMCU
// board).
// Note: GPIO 16 won't work on the ESP8266 as it does not have interrupts.
const uint16_t kRecvPin = 14;
const int LED = 15;
const int BUZZER = 5;

IRrecv irrecv(kRecvPin);

decode_results results;

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  irrecv.enableIRIn();  // Start the receiver
  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50);
  Serial.println();
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(kRecvPin);
}

void blink(int count, int interval) {
  for(int i=0; i < count; i++) {
    digitalWrite(LED, HIGH);
    delay(interval);
    digitalWrite(LED, LOW);
    delay(interval);
  }
}

void loop() {
  if (irrecv.decode(&results)) {
    // print() & println() can't handle printing long longs. (uint64_t)
    serialPrintUint64(results.value, HEX);
    Serial.println("");

    if (results.value == 0xFFC23D) {
      Serial.println("GOT PLAY!");
      blink(3, 150);
    }
    
    irrecv.resume();  // Receive the next value
  }
  delay(100);
}

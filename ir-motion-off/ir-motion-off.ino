const int LED = 2;
const int RELAY = 6;
const int IR = 7;
unsigned long SECOND = 1000L;
unsigned long MINUTE = SECOND * 60;
unsigned long WARM_UP_DELAY = MINUTE;  // 1 minute.
unsigned long ON_DELAY = MINUTE * 3;   // After off by motion.
unsigned long lastMovement = 0;
int motion = 0;
int state = LOW;
int lastMotionTime = 0;

void blink(int blinks, int blinkDelay = 100) {
  for (int i = 0; i < blinks; i++) {
    digitalWrite(LED, HIGH);
    delay(blinkDelay);

    digitalWrite(LED, LOW);
    delay(blinkDelay);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Wait 1 min for activation (warm up)...");
  pinMode(LED, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(IR, INPUT);
  blink(1);
  delay(WARM_UP_DELAY);
  blink(3);
  Serial.println("ACTIVATED!");
}

// Relay off.
// Led is ON when relay is off.
// Relay is NC, HIGH turns it off.
void off() {
  digitalWrite(LED, HIGH);
  digitalWrite(RELAY, HIGH);
  state = HIGH;
}

// Relay on. Led off.
void on() {
  digitalWrite(LED, LOW);
  digitalWrite(RELAY, LOW);
  state = LOW;
}

void loop() {
  motion = digitalRead(IR);
  Serial.print("motion:");
  Serial.println(motion);
  unsigned long currentTime = millis();
  unsigned long elapsed = currentTime - lastMovement;

  if (motion == 1) {
    lastMovement = millis();

    // Turn off relay.
    if (state == HIGH) return;  // Already off, skip.
    off();
    return;
  }

  // Turn on relay after a given interval without movement.
  if (motion == 0 && elapsed > ON_DELAY) {
    if (state == LOW) return;  // Already on, skip.
    on();
  }
}

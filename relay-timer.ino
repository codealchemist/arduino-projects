//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <LiquidCrystal_I2C.h>
#include <arduino-timer.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
auto timer = timer_create_default();

const int RELAY = 8;
const int LED = 9;
const int BUTTON_LEFT = 10;
const int BUTTON_MID = 11;
const int BUTTON_RIGHT = 12;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 250;
int inputEnabled = 1;
int isRunning = 0;
int showSeconds = 0;
long SECOND = 1000;
long MINUTE = SECOND * 60;
long HOUR = MINUTE * 60;
const long TIME_BLOCK = MINUTE * 15;
const long MIN_TIME = MINUTE * 15;
const long START_TIME = HOUR * 2;
const long MAX_TIME = HOUR * 24;
long timeLeft = START_TIME;

String getReadableTime(unsigned long ms) {
  String readableTime = "";

  int hours = (int)(ms / 1000 / 60 / 60);
  int minutes = ms % HOUR / 1000 / 60;
  int seconds = (ms - (hours * HOUR) - (minutes * MINUTE)) / 1000;

  Serial.print("--- ms:");
  Serial.println(ms);
  Serial.print("HOURS:");
  Serial.println(hours);
  Serial.print("MINUTES:");
  Serial.println(minutes);
  Serial.print("SECONDS:");
  Serial.println(seconds);

  // If it's the last minute display seconds instead.
  if (ms >= MINUTE) {
    readableTime.concat(hours);
    readableTime.concat(":");
    if (minutes < 10) readableTime.concat(0);
    readableTime.concat(minutes);
    
    if (showSeconds == 1) {
      readableTime.concat(":");
      if (seconds < 10) readableTime.concat(0);
      readableTime.concat(seconds);
    }
  } else {
    readableTime.concat(seconds);
  }
  
  return readableTime;
}

String lastReadableTime = getReadableTime(timeLeft);

void setup() {
  Serial.begin(9600);
  pinMode(LED, OUTPUT);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_MID, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
   
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Hola Bruja!");
  lcd.setCursor(0,1);
  lcd.print(":)");
  delay(3000);
  updateTime();

  timer.every(1000, [] {
    if (isRunning == 1) {
      timeLeft = timeLeft - SECOND;
  
      String currentReadableTime = getReadableTime(timeLeft);
      if (currentReadableTime != lastReadableTime) {
        updateTime();
        lastReadableTime = currentReadableTime;
      }
    }
  });
}

void updateTime() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("TIME: ");
  String readableTimeLeft = getReadableTime(timeLeft);
  lcd.print(readableTimeLeft);

  if (timeLeft <= 0) {
    isRunning = 0;
    lcd.setCursor(0,1);
    lcd.print("--> TIME is UP!");
    delay(3000);
    timeLeft = START_TIME;
    updateTime();
    return;
  }

  if (isRunning) {
    lcd.setCursor(0,1);
    lcd.print(">>> RUNNING <<<");
  }
}

void setPaused() {
  lcd.setCursor(0,1);
  lcd.print("...  PAUSED  ...");
}

void loop() {
  timer.tick();
  int buttonLeft = digitalRead(BUTTON_LEFT);
  int buttonMid = digitalRead(BUTTON_MID);
  int buttonRight = digitalRead(BUTTON_RIGHT);
  unsigned long currentTime = millis();

  // Enable input after debounce delay passed.
  if (inputEnabled == 0) {
    if (currentTime - lastDebounceTime > debounceDelay) {
      inputEnabled = 1;
    } 
  }

  // Reset time if left & right buttons are pressed at the same time.
  if (buttonLeft == LOW && buttonRight == LOW && inputEnabled == 1) {
    inputEnabled = 0;
    isRunning = 0;
    timeLeft = START_TIME;
    updateTime();
    lastDebounceTime = millis();
    return;
  }

  // Lower time when left button is pushed.
  if (buttonLeft == LOW && inputEnabled == 1) {
    inputEnabled = 0;
    timeLeft = timeLeft - TIME_BLOCK;

    // Do not allow going beyond min time.
    if (timeLeft < MIN_TIME) timeLeft = MIN_TIME;
    
    updateTime();
    lastDebounceTime = millis();
  }

  // Start / stop when middle button is pressed.
  if (buttonMid == LOW && inputEnabled == 1) {
    inputEnabled = 0;
    
    if (isRunning == 1) {
      isRunning = 0;
      setPaused();
      digitalWrite(LED, LOW);
      digitalWrite(RELAY, LOW);
      Serial.println("> PAUSED");
    } else {
      isRunning = 1;
      updateTime();
      digitalWrite(LED, HIGH);
      digitalWrite(RELAY, HIGH);
      Serial.println("> RUNNING");
    }

    lastDebounceTime = millis();
  }

  // Raise time when right button is pushed.
  if (buttonRight == LOW && inputEnabled == 1) {
    inputEnabled = 0;
    timeLeft = timeLeft + TIME_BLOCK;

    // Do not allow going beyond min time.
    if (timeLeft > MAX_TIME) timeLeft = MAX_TIME;
    
    updateTime();
    lastDebounceTime = millis();
  }
}

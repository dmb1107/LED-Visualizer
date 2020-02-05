#include <FastLED.h>
#define NUM_LEDS 60
#define MIC_PIN A5
#define LED_PIN 6
#define COLOR_ORDER GRB
#define BTN_PIN 0

CRGB leds[NUM_LEDS];

// Button stuff
int mode = 0;
int buttonState = 0;
int lastButtonState = 0;
unsigned long btnCounter = 0;
const int NUM_MODES = 6;
const int BTN_DEBOUNCE = 10;
const unsigned long SHORT_PRESS = 100;
const unsigned long LONG_PRESS = 1000;

void setup() {
  pinMode(BTN_PIN, INPUT);
  digitalWrite(BTN_PIN, HIGH);
  Serial.begin(57600);

  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  clearLEDS();

}

void loop() {
  checkButtons();

  switch(mode){
    case -1:
      clearLEDS();
      break;
    case 0:
      showWave(10, CRGB::Green, CRGB::Red, 50);
      break;
    case 1:
      showWave(10, CRGB::Red, CRGB::Orange, 50);
      break;
    case 2:
      showWave(10, CRGB::Orange, CRGB::Yellow, 50);
      break;
    case 3:
      showWave(10, CRGB::Yellow, CRGB::Pink, 50);
      break;
    case 4:
      showWave(10, CRGB::Pink, CRGB::Purple, 50);
      break;
    case 5:
      showWave(10, CRGB::Purple, CRGB::Green, 50);
      break;
    default:
      showWave(10, CRGB::Green, CRGB::Blue, 50);
      break;
  }
}

// Sets LED strip to all random colors
void setRandomColors() {
  randomSeed(analogRead(5));
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(random(255), random(255), random(255));
  }
}

void clearLEDS(){
  FastLED.clear();
  FastLED.show();
}

// Sends waveLen size wave of fgColor color with a delayTime delay
// Also changes background color
void showWave(int waveLen, CRGB fgColor, CRGB bgColor, int delayTime) {
  for (int i = 0; i < NUM_LEDS; i++) {
    for (int j = 0; j < waveLen; j++) {
      leds[(i + j) % NUM_LEDS] = fgColor;
    }
    leds[i] = bgColor;
    FastLED.show();
    // If delay is aborted due to button press, clear and return
    if(!myDelay(delayTime)){ clearLEDS(); return; }
  }
}

// Check button presses during delay
// Returns false if delay is aborted
bool myDelay(unsigned long duration) {
  unsigned long start = millis();

  while (millis() - start <= duration) {
    bool pressed = checkButtons();
    if(pressed){
      return false;
    }
  }
  return true;
}

// Updates buttons
// Returns true if event is changed
bool checkButtons() {
  bool returnVal = false;
  buttonState = digitalRead(BTN_PIN);
  // Has button state changed?
  if (buttonState != lastButtonState) {
    delay(BTN_DEBOUNCE);
    // Update in case of bounce
    buttonState = digitalRead(BTN_PIN);
    if (buttonState == LOW) {
      // Button pressed
      btnCounter = millis();
    }
    if (buttonState == HIGH) {
      // No longer pressed
      unsigned long currentMillis = millis();
      if ((currentMillis - btnCounter >= SHORT_PRESS) && !(currentMillis - btnCounter >= LONG_PRESS)) {
        // Short press detected
        Serial.println("Short");
        mode = mode == -1 ? -1 : (mode + 1) % NUM_MODES;
        returnVal = true;
        Serial.println(mode);
      }
      if (currentMillis - btnCounter >= LONG_PRESS) {
        // Long press detected
        Serial.println("Long");
        // Turn strips on or off
        mode = mode == -1 ? 0 : -1;
        returnVal = true;
        Serial.println(mode);
      }
    }
  }
  lastButtonState = buttonState;
  return returnVal;
}

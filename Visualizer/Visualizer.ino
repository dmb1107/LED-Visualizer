#include <FastLED.h>
#define NUM_LEDS 60
#define MIC_PIN A5
#define LED_PIN 6
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

}

void loop() {
    showWave(10, CRGB::Green, CRGB::Blue, 50);
    
}

// Sets LED strip to all random colors
void setRandomColors() {
  randomSeed(analogRead(5));
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(random(255), random(255), random(255));
  }
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
    delay(delayTime);
  }
}


// Unused
void setLEDShift() {
  CRGB tmp = leds[NUM_LEDS - 1];
  for (int i = NUM_LEDS - 1; i > 0; i--) {
    leds[i] = leds[i - 1];
  }
  leds[0] = tmp;
}

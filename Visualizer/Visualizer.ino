//#include <Adafruit_NeoPixel.h>
#include <FastLED.h>
#define NUM_LEDS 60
#define MIC_PIN A5
#define LED_PIN 6
#define COLOR_ORDER GRB


CRGB leds[NUM_LEDS];
//Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);


void setup() {
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
//  strip.begin();
}

void loop() {
  leds[0] = CRGB::Red; 
  FastLED.show(); 
  delay(30); 
}

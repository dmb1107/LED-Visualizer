#define FASTLED_ESP8266_RAW_PIN_ORDER

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "FastLED.h"

#define NUM_LEDS 60
#define LED_PIN 5
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

// Modes
#define OFF 1
#define STATIC 2
#define WAVES 3
#define MUSIC 4

// Wave Modes
#define FLOW 1
#define FALL 2
#define RAINBOW 3


CRGB leds[NUM_LEDS];

// Default values to off
int r = 0, g = 0, b = 0;

int mode = OFF;
int waveMode = FLOW; // Wave sub-mode

// Blynk auth
char auth[] = "KDzz0yZ-CWm38Xll10KSb6lqGKLIuKMs";

// WiFi credentials
char ssid[] = "A Hint of Lime 2.4GHz";
char pass[] = "BeIrJoMaOl69420";

void setup()
{
  // Debug console
  Serial.begin(9600);

  // Set up LEDs
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);

  // Connect to Blynk
  Blynk.begin(auth, ssid, pass);
}

void loop()
{
  // Check incoming Blynk commands
  Blynk.run();

  switch (mode) {
    case OFF: {
        setAllLEDS(0, 0, 0);
        FastLED.show();
        break;
      }
    case STATIC: {
        staticColor(r, g, b);
        break;
      }
    case WAVES: {
        waves();
        break;
      }
    case MUSIC: {
        break;
      }
  }
}

// Mode 3
void waves() {
  switch (waveMode) {
    case FLOW:
      flow();
      break;
    case FALL:
      fall(0xff, 0xff, 0xff, 100); // white
      break;
    case RAINBOW:
      rainbowCycle(5);
      break;
  }
}

// waveMode 2
void fall(byte red, byte green, byte blue, int WaveDelay) {
  int Position = 0;
  for (int j = 0; j < NUM_LEDS * 2; j++) {
    Position++; // = 0; //Position + Rate;
    for (int i = 0; i < NUM_LEDS; i++) {
      // sine wave, 3 offset waves make a rainbow!
      float level = sin(i + Position) * 127 + 128;
      leds[i] = CRGB(level / 255 * red, level / 255 * green, level / 255 * blue);
    }
    FastLED.show();
    blynk_delay(WaveDelay);
    if (mode != WAVES || waveMode != FALL) { return; }
  }
}

// waveMode 3
void rainbowCycle(int SpeedDelay) {
  byte *c;
  uint16_t i, j;

  for(j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for(i = 0; i < NUM_LEDS; i++) {
      c = Wheel(((i * 256 / NUM_LEDS) + j) & 255);
      leds[i] = CRGB(*c, *(c + 1), *(c + 2));
    }
    FastLED.show();
    blynk_delay(SpeedDelay);
    if (mode != WAVES || waveMode != RAINBOW) { return; }
  }
}


// Updates LEDS to static color
// Mode 2
void staticColor(int red, int green, int blue) {
  // Change colors
  setAllLEDS(red, green, blue);
  FastLED.show();
}

// sets all LEDS to given color, Not displayed
void setAllLEDS(int red, int green, int blue) {
  fill_solid(leds, NUM_LEDS, CRGB(red, green, blue));
}

// Wave Mode flow
void flow() {
  for (int j = 0; j < 3; j++ ) {
    // Fade IN
    for (int k = 0; k < 256; k++) {
      switch (j) {
        case 0: {
            setAllLEDS(k, 0, 0);
            FastLED.show();
            break;
          }
        case 1: {
            setAllLEDS(0, k, 0);
            FastLED.show();
            break;
          }
        case 2: {
            setAllLEDS(0, 0, k);
            FastLED.show();
            break;
          }
      }
      FastLED.show();
      blynk_delay(3);
      if (mode != WAVES || waveMode != FLOW) { return; }
    }
    // Fade OUT
    for (int k = 255; k >= 0; k--) {
      switch (j) {
        case 0: {
            setAllLEDS(k, 0, 0);
            FastLED.show();
            break;
          }
        case 1: {
            setAllLEDS(0, k, 0);
            FastLED.show();
            break;
          }
        case 2: {
            setAllLEDS(0, 0, k);
            FastLED.show();
            break;
          }
      }
      FastLED.show();
      blynk_delay(3);
      if (mode != WAVES || waveMode != FLOW) { return; }
    }
  }
}

// Mode Picker
BLYNK_WRITE(V0) {
  mode = param.asInt();
}

// Wave-Mode Picker
BLYNK_WRITE(V1) {
  waveMode = param.asInt();
}

// RGB Control
BLYNK_WRITE(V3) {
  if (mode == STATIC) {
    r = param[0].asInt();
    g = param[1].asInt();
    b = param[2].asInt();
    staticColor(r, g, b);
  }
}

byte * Wheel(byte WheelPos) {
  static byte c[3];
  if (WheelPos < 85) {
    c[0] = WheelPos * 3;
    c[1] = 255 - WheelPos * 3;
    c[2] = 0;
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    c[0] = 255 - WheelPos * 3;
    c[1] = 0;
    c[2] = WheelPos * 3;
  } else {
    WheelPos -= 170;
    c[0] = 0;
    c[1] = WheelPos * 3;
    c[2] = 255 - WheelPos * 3;
  }
  return c;
}

// Delay without stopping processes
void blynk_delay(int milli) {
  int end = millis() + milli;
  while (millis() < end) {
    Blynk.run();
  }
}

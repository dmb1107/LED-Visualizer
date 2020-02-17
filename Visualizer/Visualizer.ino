#define FASTLED_ESP8266_RAW_PIN_ORDER

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "FastLED.h"
#include "arduinoFFT.h"

#define NUM_LEDS 60
#define LED_PIN 5
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

#define MIC_PIN 0 // A0

// Modes
#define OFF 1
#define STATIC 2
#define WAVES 3
#define MUSIC 4

// Wave Modes
#define FLOW 1
#define FALL 2
#define RAINBOW 3

// Music Modes
#define VOLUME 1
#define FREQ_1 2

// Sampling
#define SAMPLES 256             //Must be a power of 2
#define SAMPLING_FREQUENCY 10000 //Hz, must be less than 10000 due to ADC
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
arduinoFFT FFT = arduinoFFT();
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned int sampling_period_us;
unsigned long microseconds;

// LED strip
CRGB leds[NUM_LEDS];

// Current static colors
int r, g, b;

// Default mode and wave-mode values
int mode = OFF;
int waveMode = FLOW; // Wave sub-mode
int musicMode = VOLUME; // Music sub-mode

// Blynk auth
char auth[] = "KDzz0yZ-CWm38Xll10KSb6lqGKLIuKMs";

// WiFi credentials
char ssid[] = "A Hint of Lime 2.4GHz";
char pass[] = "BeIrJoMaOl69420";

void setup()
{
  // Debug console
  Serial.begin(9600);

  // Set up FastLED LED strip
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);

  // Connect to Blynk
  Blynk.begin(auth, ssid, pass);

  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
}

void loop()
{
  // Check incoming Blynk commands
  Blynk.run();

  switch (mode) {
    case OFF:
      // Set LEDS to black
      setAllLEDS(0, 0, 0);
      FastLED.show();
      break;
    case STATIC:
      // Set LEDS to static RGB values
      staticColor(r, g, b);
      break;
    case WAVES:
      waves();
      break;
    case MUSIC:
      musicReactive();
      break;
  }
}

/*
  Section: Modes
*/

// Static Mode
void staticColor(int red, int green, int blue) {
  setAllLEDS(red, green, blue);
  FastLED.show();
}

// Waves Mode
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

// Music mode
void musicReactive() {
  switch (musicMode) {
    case VOLUME:
      volume();
      break;
    case FREQ_1:
      freq_1();
      break;
  }
}

/*
   Section: Music Modes
*/

// Music mode -> volume
void volume() {
  double volts = readMic();
  if (volts == -1) {
    return;
  }
  Serial.println(volts);
  double pixelNum = (NUM_LEDS / 4.95) * volts; // 4.95 is max volts, so normalize to NUM_LEDS

  // Cap at 60 just in case
  if (pixelNum >= NUM_LEDS) {
    pixelNum = NUM_LEDS;
  }

  // Set pixels to color
  for (int i = 0; i < pixelNum; i++) {
    leds[i] = CRGB::Blue;
  }

  // Clear other LEDS
  for (int i = pixelNum; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }

  FastLED.show();
}

// Music mode -> freq_1
void freq_1() {
  for (int i = 0; i < SAMPLES; i++)
  {
    microseconds = micros();    //Overflows after around 70 minutes!

    vReal[i] = analogRead(0);
    vImag[i] = 0;

    while (micros() < (microseconds + sampling_period_us)) {
    }
  }

  /*FFT*/
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

  int countB = 0;
  int countM = 0;
  int countH = 0;
    
  for (int i = 2; i < (SAMPLES / 2); i++)
  {
    yield();
    if(mode != MUSIC && musicMode != FREQ_1) return;
    double frequency = (i * 1.0 * SAMPLING_FREQUENCY) / SAMPLES;
    
    if(vReal[i] > 500){
      if(frequency > 50 && frequency < 80){
        countB++;
        double val = vReal[i] / 2000.0;
        freq_1_Band(0, (int)(val * 30));
      } else if(frequency > 150 && frequency < 2000){
        // TODO: Find peak vReal in frequency range and only display band on that one
        countM++;
        double val = vReal[i] / 2000.0;
        freq_1_Band(1, (int)(val * 27));
      }
      else if(frequency > 2000){
        if(vReal[i] > 15000){
          countH++;
          freq_1_Band(2, 3);
        }
      }
    }
  }
  if(countB == 0){
    for(int i = 3; i < 30; i++){
      leds[i] = CRGB::Black;
    }
  }
  if(countM == 0){
    for(int i = 30; i < NUM_LEDS - 3; i++){
      leds[i] = CRGB::Black;
    }
  }
  if(countH == 0){
    leds[0] = CRGB::Black;
    leds[1] = CRGB::Black;
    leds[2] = CRGB::Black;
    
    leds[NUM_LEDS - 1] = CRGB::Black;
    leds[NUM_LEDS - 2] = CRGB::Black;
    leds[NUM_LEDS - 3] = CRGB::Black;
  }
  FastLED.show();
  blynk_delay(4);
}

/*
  Section: Wave Modes
*/

// Wave Mode -> Fall
// Modified from: https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectRunningLights
void fall(byte red, byte green, byte blue, int WaveDelay) {
  int Position = 0;
  for (int j = 0; j < NUM_LEDS * 2; j++) {
    Position++;
    for (int i = 0; i < NUM_LEDS; i++) {
      float level = sin(i + Position) * 127 + 128;
      leds[i] = CRGB(level / 255 * red, level / 255 * green, level / 255 * blue);
    }
    FastLED.show();
    blynk_delay(WaveDelay);
    if (mode != WAVES || waveMode != FALL) {
      return;
    }
  }
}

// Wave Mode -> RainbowCycle
// Modified from: https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectRainbowCycle
void rainbowCycle(int SpeedDelay) {
  byte *c;
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < NUM_LEDS; i++) {
      c = Wheel(((i * 256 / NUM_LEDS) + j) & 255);
      leds[i] = CRGB(*c, *(c + 1), *(c + 2));
    }
    FastLED.show();
    blynk_delay(SpeedDelay);
    if (mode != WAVES || waveMode != RAINBOW) {
      return;
    }
  }
}

// Wave Mode -> Flow
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
      if (mode != WAVES || waveMode != FLOW) {
        return;
      }
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
      if (mode != WAVES || waveMode != FLOW) {
        return;
      }
    }
  }
}

/*
  Section: Blynk
*/

// Mode Picker
BLYNK_WRITE(V0) {
  mode = param.asInt();
  if (mode == STATIC) {
    Blynk.syncVirtual(V3); // Sync RGB control
  }
}

// Wave-Mode Picker
BLYNK_WRITE(V1) {
  waveMode = param.asInt();
}

// Music-Mode Picker
BLYNK_WRITE(V4) {
  musicMode = param.asInt();
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

// Sync when Blynk connects
BLYNK_CONNECTED() {
  Blynk.syncAll();
}

// Delay without stopping processes
void blynk_delay(int milli) {
  int end = millis() + milli;
  while (millis() < end) {
    Blynk.run();
  }
}

/*
  Section: Helpers
*/

// Sets all LEDS to given color, Not displayed
void setAllLEDS(int red, int green, int blue) {
  fill_solid(leds, NUM_LEDS, CRGB(red, green, blue));
}

// Reads mic sample and returns voltage
// If mode is changed during execution, returns -1
double readMic() {
  unsigned long startMillis = millis(); // Start of sample window
  unsigned int peakToPeak = 0;   // peak-to-peak level

  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  // Collect data for 50 mS
  while (millis() - startMillis < sampleWindow)  {
    sample = analogRead(MIC_PIN);
    blynk_delay(4);    //Need this delay otherwise esp8266 gets locked up.
    if (mode != MUSIC) {
      return -1;
    }
    yield();
    if (sample < 1024)  {
      if (sample > signalMax) {
        signalMax = sample;  // save just the max levels
      }
      else if (sample < signalMin) {
        signalMin = sample;  // save just the min levels
      }
    }
  }
  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
  double volts = (peakToPeak * 5.0) / 1024;  // convert to volts
  return volts;
}

void freq_1_Band(int band, int len){
  if(band == 0){
    if(len > 27) len = 27;
    for(int i = 3; i < 30; i++){
      if(i > len){
        leds[i] = CRGB::Blue;
      } else {
        leds[i] = CRGB::Black;
      }
    }
  } else if(band == 1){
    if(len > 27) len = 27;
    for(int i = 30; i < NUM_LEDS - 3; i++){
      if(i < 20 + len){
        leds[i] = CRGB::Green;
      } else {
        leds[i] = CRGB::Black;
      }
    }
  } else {
    leds[0] = CRGB::White;
    leds[1] = CRGB::White;
    leds[2] = CRGB::White;
    
    leds[NUM_LEDS - 1] = CRGB::White;
    leds[NUM_LEDS - 2] = CRGB::White;
    leds[NUM_LEDS - 3] = CRGB::White;
  }
}

// Color wheel
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

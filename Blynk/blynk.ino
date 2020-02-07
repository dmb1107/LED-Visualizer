/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "FastLED.h"

#define NUM_LEDS 60
//#define MIC_PIN A5
#define LED_PIN D1
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
//#define BTN_PIN 0

CRGB leds[NUM_LEDS];

int r, g, b;
int data = 255;

// Blynk auth
char auth[] = "KDzz0yZ-CWm38Xll10KSb6lqGKLIuKMs";

// WiFi credentials
char ssid[] = "A Hint of Lime 2.4GHz";
char pass[] = "BeIrJoMaOl69420";


// Attach virtual serial terminal to Virtual Pin V1
WidgetTerminal terminal(V1);

void setup()
{
  // Debug console
  Serial.begin(9600);

  // Connect to Blynk
  Blynk.begin(auth, ssid, pass);

  // Set up LEDs
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
}


void loop()
{
  // Check incoming Blynk commands
  Blynk.run();
}

void staticColor(int r, int g, int b, int brightness){
  FastLED.setBrightness(brightness);
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB(r, g, b);
  }
  FastLED.show();
}

// RGB Control
BLYNK_WRITE(V3){
  r = param[0].asInt();
  g = param[1].asInt();
  b = param[2].asInt();
  staticColor(r, g, b, data);
}

// Brightness control
BLYNK_WRITE(V2){
  data = param.asInt();
  staticColor(r, g, b, data);
}

// Terminal
BLYNK_WRITE(V1)
{

  // if you type "Marco" into Terminal Widget - it will respond: "Polo:"
  if (String("Marco") == param.asStr()) {
    terminal.println("You said: 'Marco'") ;
    terminal.println("I said: 'Polo'") ;
  } else {

    // Send it back
    terminal.print("You said:");
    terminal.write(param.getBuffer(), param.getLength());
    terminal.println();
  }

  // Ensure everything is sent
  terminal.flush();
}

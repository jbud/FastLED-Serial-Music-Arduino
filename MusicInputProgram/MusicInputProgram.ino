#include "FastLED.h"
#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    9
//#define CLK_PIN   4
#define LED_TYPE    TM1803
#define COLOR_ORDER GBR // Reversed for styling. My default is GBR
#define NUM_LEDS    10
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          36
#define FRAMES_PER_SECOND  120 // This limit can improve performance and reduce the chance of overflow lag.
#define AVG                true

uint8_t gHue = 0;
uint8_t gOffset = 0;
byte temp;

void setup()
{
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  Serial.begin(57600);
}
void rainbow()
{
  fill_rainbow( leds, NUM_LEDS, gHue, 0); // Create a rainbow and adjust based on global hue.
}
byte limit(byte in) {
  if (in > 128) {
    temp = in - 127;
    temp = temp * 2;
  }
  else {
    temp = 0;
  }
  return round(in); // Bypassing this function until better math is performed... -_-
}
int lvl = 1;      // Level of spectrum to analyze
int data[9];      // raw serial data array
int isData = 0;
uint16_t averg = 0;
byte processedData = 0;
void loop() {
  if (Serial.available() > 10) {
    byte i = Serial.read();
    if (int(i) == 255) {
      for (int c = 0; c < 9; c++) {
        data[c] = int(Serial.read()); // Build array with all levels of data (even though we only use one at a time now, the rest of the spectral data can be used for further programming later)
        if (data[c] != 0){
          isData = 1;
        }
      }
      processedData = data[lvl];
    }
  }
  if (AVG){
    for (int j = 0; j <= 9; j++){
      averg += data[j];
    }
    averg = round(averg / 9);
    processedData = averg;
    if (processedData < 10){
      processedData = 10; 
    }
  }
  if (isData == 1) {
    FastLED.setBrightness(processedData);
    gHue = processedData + gOffset;
    rainbow();
    FastLED.show();
    EVERY_N_MILLISECONDS( 40 ) {
      gOffset++;
    }
  }
  else { // If serial data is idle just show a standard rotating rainbow.
    FastLED.setBrightness(BRIGHTNESS); // Set brightness to defined level when idle
    rainbow();
    FastLED.show();
    EVERY_N_MILLISECONDS( 20 ) {
      gHue++;
    }
  }
  FastLED.delay(1000 / FRAMES_PER_SECOND); // Limit to FPS rate.
}

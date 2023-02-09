#include <Adafruit_TinyUSB.h>
#include <Adafruit_NeoPixel.h>

const uint16_t PixelCount = 1;
const uint8_t PixelPin = PIN_NEOPIXEL;

// Brightness in this case. 255 is max. 32 is visually palatable.
#define colorSaturation 32

// Define 3-pixel strip PixelCount long, using PixelPin as the DIN. Use GRB format formatted for WS2812.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PixelPin, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for serial attach

  Serial.println();
  Serial.println("Initializing...");
  Serial.flush();

  // this resets all the neopixels to an off state
  strip.begin();
  strip.setBrightness(50);
  strip.show();


  Serial.println();
  Serial.println("Running...");
}

void loop()
{
  rainbow(20);
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

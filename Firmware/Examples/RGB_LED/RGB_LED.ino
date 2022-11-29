#include <NeoPixelBus.h> // http://librarymanager/All#NeoPixelBus

const uint16_t PixelCount = 1;
const uint8_t PixelPin = LEDRGB;

// Brightness in this case. 255 is max. 32 is visually palatable.
#define colorSaturation 32

// Define 3-pixel strip PixelCount long, using PixelPin as the DIN. Use GRB format formatted for WS2812.
NeoPixelBus<NeoGrbFeature, NeoWs2812Method> strip(PixelCount, PixelPin);

// Define colors using RGB format.
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

// Define colors using Hue, Saturation, Lightness (HSL) format.
HslColor hslRed(red);
HslColor hslGreen(green);
HslColor hslBlue(blue);
HslColor hslWhite(white);
HslColor hslBlack(black);

void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for serial attach

  Serial.println();
  Serial.println("Initializing...");
  Serial.flush();

  // this resets all the neopixels to an off state
  strip.Begin();
  strip.Show();


  Serial.println();
  Serial.println("Running...");
}

void loop()
{
  delay(2000);

  Serial.println("Colors R, G, B, W...");

  // set the colors, 
  // if they don't match in order, you need to use NeoGrbFeature feature
  strip.SetPixelColor(0, red);
  strip.Show();
  delay(500);
  strip.SetPixelColor(0, green);
  strip.Show();
  delay(500);
  strip.SetPixelColor(0, blue);
  strip.Show();
  delay(500);
  strip.SetPixelColor(0, white);
  strip.Show();
  delay(500);

  delay(500);

  Serial.println("Off ...");

  // turn off the pixels
  strip.SetPixelColor(0, black);
  strip.Show();

  delay(1000);

  Serial.println("HSL Colors R, G, B, W...");

  // set the colors, 
  // if they don't match in order, you may need to use NeoGrbFeature feature
  strip.SetPixelColor(0, hslRed);
  strip.Show();
  delay(500);
  strip.SetPixelColor(0, hslGreen);
  strip.Show();
  delay(500);
  strip.SetPixelColor(0, hslBlue);
  strip.Show();
  delay(500);
  strip.SetPixelColor(0, hslWhite);
  strip.Show();
  delay(500);    


  delay(500);

  Serial.println("Off again...");

  // turn off the pixels
  strip.SetPixelColor(0, hslBlack);
  strip.Show();
  delay(1000);
}

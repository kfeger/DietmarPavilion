/*
All die Funktionen, die buntes Licht machen...
Grundlage ist die Adafruit WS2801-Library
*/

void rainbow(uint8_t wait) {
  int i, j;
  while (1) {
    for (j = 0; j < 256; j++) {   // 3 cycles of all 256 colors in the wheel
      for (i = 0; i < STRIPLEN; i++) {
        if (RC5.FreshData)
          return;
        LEDStrip.setPixelColor(i, Wheel( (i + j) % 255));
      }
      LEDStrip.show();   // write all the pixels out
      delay(wait);
    }
  }
}

// Slightly different, this one makes the rainbow wheel equally distributed
// along the chain
void rainbowCycle(uint8_t wait) {
  int i, j;

  while (1) {
    for (j = 0; j < 256 * 5; j++) {   // 5 cycles of all 25 colors in the wheel
      for (i = 0; i < STRIPLEN; i++) {
        if (RC5.FreshData)
          return;
        LEDStrip.setPixelColor(i, Wheel( ((i * 256 / STRIPLEN) + j) % 256) );
      }
      LEDStrip.show();   // write all the pixels out
      delay(wait);
    }
  }
}

void rotateStrip(uint16_t wait) {
  //uint32_t TempColor = 0;
  while (1) {
    uint32_t LastColor = LEDStrip.getPixelColor(STRIPLEN - 1);
    for (uint16_t i = (STRIPLEN - 1); i ; i--) {
      LEDStrip.setPixelColor(i, LEDStrip.getPixelColor(i - 1));
    }
    LEDStrip.setPixelColor(0, LastColor);
    LEDStrip.show();
    if (RC5.FreshData)
      return;
    delay(wait);
  }
}


// fill the dots one after the other with said color
// good for testing purposes
void colorFill(uint32_t c) {
  for (uint8_t i = 0; i < STRIPLEN; i++) {
    LEDStrip.setPixelColor(i, c);
  }
  LEDStrip.show();
}


// fill the dots one after the other with said color
// good for testing purposes
void colorFillSlow(uint32_t c, uint16_t wait) {
  for (uint8_t i = 0; i < STRIPLEN; i++) {
    LEDStrip.setPixelColor(i, c);
  }
  LEDStrip.show();
  delay(wait);
}


void randomFill(void) {
  for (uint8_t i = 0; i < STRIPLEN; i++) {
    Red = random(1, 256);
    Green = random(1, 256);
    Blue = random(1, 256);
    LEDStrip.setPixelColor(i, Color(Red, Green, Blue));
  }
  LEDStrip.show();
}

void PickRandomBase(void) {
  uint8_t ColorPick = (unsigned char)random(1, 7);
  uint8_t Bright = 255;
  switch (ColorPick) {
    case 1:
      Red = Bright;
      Green = 0;
      Blue = 0;
      break;
    case 2:
      Red = 0;
      Green = Bright;
      Blue = 0;
      break;
    case 3:
      Red = Bright;
      Green = Bright;
      Blue = 0;
      break;
    case 4:
      Red = 0;
      Green = 0;
      Blue = Bright;
      break;
    case 5:
      Red = Bright;
      Green = 0;
      Blue = Bright;
      break;
    case 6:
      Red = 0;
      Green = Bright;
      Blue = Bright;
      break;
    case 7:
      Red = Bright;
      Green = Bright;
      Blue = Bright;
      break;
    default:
      Red = 128;
      Green = 128;
      Blue = 128;
      break;
  }
}

void randomFillRGB(void) {
  for (uint8_t i = 0; i < STRIPLEN; i++) {
    PickRandomBase();
    LEDStrip.setPixelColor(i, Color(Red, Green, Blue));
  }
  LEDStrip.show();
}

/* Helper functions */

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85) {
    return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

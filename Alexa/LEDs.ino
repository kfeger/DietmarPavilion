/*
 * Steuerung der LED-Strips
 */
 void FillLEDsBlack(void) {
  FastLED.clear();
  FastLED.show();
}

void FillLedsHSV(byte hue, byte sat, byte val) {
  for(int i; i < NUM_LEDS; i++) {
    leds[i].setHSV( hue, sat, val);
  }
  FastLED.show();  
}

void FillLedsColor(void) {
  MessBeginn = millis();
  for(int i; i < NUM_LEDS; i++) {
    leds[i] = CRGB(rot, gruen, blau);
    leds[i].nscale8(hell);
  }
  FastLED.show(); 
  Serial.printf("\nFillLedsColor lief %ul ms\n", millis() - MessBeginn); 
}

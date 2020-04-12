/*
 * Die eigentliche Alexa-Steuerung
 */
void GartenChanged(EspalexaDevice* garten)
{
  if (garten == nullptr) return;

  Serial.print("Garten geaendert nach Helligkeit ");
  hell = garten->getValue();
  Serial.print(hell);
  rot = garten->getR();
  Serial.print(", Farben R");
  Serial.print(rot);
  gruen =  garten->getG();
  Serial.print(", G");
  Serial.print(gruen);
  blau =  garten->getB();
  Serial.print(", B");
  Serial.println(blau);
  SaveToEE();
  Serial.println("Daten ins EEPROM gesichert");
  newColor = true;
}

void GittaChanged(EspalexaDevice* gitta)
{
  if (gitta == nullptr) return; //this is good practice, but not required

  // Hier wird der Multiplexer umgeschaltet.
  Serial.print("Gitta ist ");
  if (gitta->getValue()) {
    Serial.println("eingeschaltet"); // Auf den Arduino umschalten
    digitalWrite(GITTA_PIN, HIGH);
    digitalWrite(GITTA_LED, HIGH);
  }
  else {
    Serial.println("ausgeschaltet");
    digitalWrite(GITTA_PIN, LOW);
    digitalWrite(GITTA_LED, LOW);
    SetFromEE();
    FillLedsColor();
  }
}

void gammaChanged(EspalexaDevice* gamma) {
  long PickRGB = 0;
  if (gamma == nullptr) return;
  Serial.print("C changed to ");
  Serial.print(gamma->getValue());
  Serial.print(", colortemp ");
  Serial.print(gamma->getCt());
  Serial.print(" (");
  Serial.print(gamma->getKelvin()); //this is more common than the hue mired values
  Serial.println("K)");
  int Kelvin = gamma->getKelvin();
  if (Kelvin < 2700)
    PickRGB = Tungsten40W;
  else if ((Kelvin >= 2700) && (Kelvin <3400))
    PickRGB = Tungsten100W;
  else if ((Kelvin >= 3400) && (Kelvin < 4000))
    PickRGB = Halogen;
  else if ((Kelvin >= 4000) && (Kelvin < 4900))
    PickRGB = CarbonArc;
  else  // Kelvin >= 4900
    PickRGB = DirectSunlight;

  hell = gamma->getValue();

  Serial.printf ("Alexa hat ""gamma"" verstanden.\nPickRGB = 0x%X, Helligkeit = 0x%X\n", PickRGB, hell);

  rot = (PickRGB >> 16) & 0xFF;
  gruen = (PickRGB >> 8) & 0xFF;
  blau = PickRGB & 0xFF;
  SaveToEE();
  newColor = true;
}

/*
 * Alternative Steuerung des Pavilion durch Amazon's Alexa
 * Läuft auf WEMOS D1-Mini
 * Die Umschaltung zwischen bisheriger und neuer Steuerung erfolgt mit
 * einem 74HCT153 Multiplexer. Die Umschaltung wird durch "Gitta an" (bisherige)
 * oder "Gitta aus" (neue Steuerung) durchgeführt.
 * Der Pavilion selbst wird in Farbe ("Garten blau") und Helligkeit ("Garten 30%")
 * realisiert. Es wird eine Philps Hue emuliert.
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>      // https://github.com/tzapu/WiFiManager
#include <Espalexa.h>         // https://github.com/Aircoookie/Espalexa
#include <FastLED.h>          // https://github.com/FastLED/FastLED
#include <EEPROM.h>

#define NUM_LEDS 100
#define DATA_PIN D6
#define CLOCK_PIN D7
#define GITTA_PIN D5
#define GITTA_LED D8

CRGB leds[NUM_LEDS];
//CRGBArray<NUM_LEDS> leds;
// Farben
byte rot = 0, gruen = 0, blau = 0, hell = 0;
bool newColor = false;

// Messen
unsigned long MessBeginn;

// EEPROM
// defines für EEPROM
#define EECHK0 0  //Adresse erstes Chek-Byte
#define EECHK1 1  //Adresse zwweites Check-Byte
#define EERED  2   //Stunden öffnen
#define EEGREEN  3   //Minuten öffnen
#define EEBLUE  4   //Stunden schließen
#define EEBRIGHT  5   //Minuten schließen



bool wifiConnected = false;

// prototypes
bool connectWifi();

//callback functions
//new callback type, contains device pointer
void WeissChanged(EspalexaDevice* dev);
void BuntChanged(EspalexaDevice* dev);

Espalexa espalexa;

void setup()
{
  Serial.begin(115200);
  pinMode(GITTA_PIN, OUTPUT);
  digitalWrite(GITTA_PIN, HIGH);
  pinMode(GITTA_LED, OUTPUT);
  digitalWrite(GITTA_LED, HIGH);
  //FastLED init
  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);

  rot = 64;
  gruen = 64;
  blau = 0;
  hell = 32;
  FillLedsColor();

  WiFiManager wifiManager;
  //wifiManager.resetSettings();
  wifiManager.setAPCallback(ConfigModeDisplay);
  wifiManager.autoConnect("Garten-AP");

  FillLEDsBlack();
  // Alexa init.
  espalexa.addDevice("Garten", GartenChanged, EspalexaDeviceType::color, 127); //Dimmable device, optional 4th parameter is beginning state (here fully on)
  espalexa.addDevice("Gitta", GittaChanged, EspalexaDeviceType::onoff); //Ein-/Aus- device
  EspalexaDevice* garten = espalexa.getDevice(0); //this will get "Garten", the index is zero-based
  EspalexaDevice* einaus = espalexa.getDevice(1); //this will get "Gitta", the index is zero-based
  // EEPROM check und u.U. init.
  EEPROM.begin(128);
  Check_EE();
  SetFromEE();
  FillLedsColor();
  espalexa.begin();
}

void loop() {
  espalexa.loop();
  delay(1);
  if (newColor) {
    MessBeginn = millis();
    FillLedsColor();
    SaveToEE();
    newColor = false;
  }
}

void ConfigModeDisplay (WiFiManager *wiFiManager) {
  rot = 0;
  gruen = 64;
  blau = 64;
  hell = 32;
  FillLedsColor();
}

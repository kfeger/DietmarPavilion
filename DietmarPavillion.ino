/* NEC-Fernbedienung auf eigener Grundlage.
Es wird Timer2 für das Timing des Empfangs verwendet.
Damit sind die "Tone"-Funktionen
des Arduino nicht mehr verwendbar.
Die musik spielt in den ISR
ISR(TIMER2_OVF_vect)	//Laufzeit ca. 4µs bei 16MHz
und
ISR(PCINT2_vect)	//Laufzeit ca. 10µs bei 8MHz

Beachten: bei der Anpassung des Pins für den IR-Receiver muss
auch die Konfig des PCINT angepasst werden.
Siehe Funktion NEC_Init() in NEC_Decode
*/

#include "Adafruit_WS2801.h"
#include "SPI.h"
#include "EEPROM.h"
#include <avr/pgmspace.h>

#define RIGHT_ARROW   0xC2
#define LEFT_ARROW    0x22
#define SELECT_BUTTON 0x02
#define UP_ARROW      0x62
#define DOWN_ARROW    0xA8
#define BUTTON_0 0x4A
#define BUTTON_1 0x68
#define BUTTON_2 0x98
#define BUTTON_3 0xB0
#define BUTTON_4 0x30
#define BUTTON_5 0x18
#define BUTTON_6 0x7A
#define BUTTON_7 0x10
#define BUTTON_8 0x38
#define BUTTON_9 0x5A
#define BUTTON_STAR 0x42
#define BUTTON_HASH 0x52

volatile struct RC5_Data
{
  unsigned char Adresse;
  unsigned char Befehl;
  unsigned char Toggle;
  unsigned char FreshData;
  unsigned int Raw;
} RC5;

struct Store  //der Zustands-Speicher
{
  uint8_t eRed;
  uint8_t eGreen;
  uint8_t eBlue;
  uint8_t eBefehl;
  uint16_t eUsed;
} Status;


#define STRIPLEN 100
#define WAIT 20
#define WAIT_SHORT 2

uint8_t Red = 0, Green = 0, Blue = 0;
uint32_t LastStatus = 0;
uint16_t DotPosition = 0;

Adafruit_WS2801 LEDStrip = Adafruit_WS2801(STRIPLEN);

#define LED_STEP_ARRAY_SIZE 38
/*const PROGMEM uint8_t LEDSteps[LED_STEP_ARRAY_SIZE] = {
  0,   7,  15,  23,  31,  39,  47,  55,
 63,  71,  79,  87,  95, 103, 111, 119,
127, 135, 143, 151, 159, 167, 175, 183,
191, 199, 207, 215, 223, 231, 239, 247,
255 };*/

const PROGMEM uint8_t LEDSteps[LED_STEP_ARRAY_SIZE] = {
  0,   1,  3,  5,  7,  9,  11,  13,
  15,  17, 20,  22,  26,  30, 34, 38, 42,
  48, 54, 60, 66, 74, 82, 90, 98, 116, 132,
  148, 160, 176, 192, 207, 215, 223, 231, 239, 247,
  255
};


uint8_t LEDCurrentStep = 0;

void setup()
{
  Serial.begin(9600);
  NEC_Init();
  LEDStrip.begin();
  LEDStrip.show();
  randomSeed(analogRead(0));
  EEPROM.get(0, Status);
  if (Status.eUsed != 0x55aa) {
    Serial.println(F("Kaltstart"));
    Status.eRed = Status.eGreen = Status.eBlue = 0;
    Status.eBefehl = 0;
    Status.eUsed = 0x55aa;
    EEPROM.put(0, Status);
  }
  else
    EEPROM.get(0, Status);

  Serial.print("Status.eRed = ");
  Serial.println(Status.eRed);
  Serial.print("Status.eGreen = ");
  Serial.println(Status.eGreen);
  Serial.print("Status.eBlue = ");
  Serial.println(Status.eBlue);
  Serial.print("Status.eBefehl = ");
  Serial.println(Status.eBefehl);
  Serial.print("Status.eUsed = ");
  Serial.println(Status.eUsed, HEX);

  Serial.println(F("Geht los!"));

  RC5.Befehl = Status.eBefehl;
  Red = Status.eRed;
  Green = Status.eGreen;
  Blue = Status.eBlue;
  if (RC5.Befehl == SELECT_BUTTON)
    colorFillSlow(Color(Red, Green, Blue), 20);
  RC5.FreshData = 1;
}

void loop() {
  if ((millis() - LastStatus) > 300000) { //alle 5 min. Status ins EEPROM
    LastStatus = millis();
    Status.eBefehl = RC5.Befehl;
    Status.eRed = Red;
    Status.eGreen = Green;
    Status.eBlue = Blue;
    EEPROM.put(0, Status);
    Serial.println(F("Sichere Daten alle 5 min."));
  }
  if (RC5.FreshData) {
    RC5.FreshData = 0;
    switch (RC5.Befehl) {
      case UP_ARROW:
        Status.eBefehl = RC5.Befehl;
        Serial.println(F("Rauf"));
        for (; LEDCurrentStep < LED_STEP_ARRAY_SIZE; LEDCurrentStep++) {
          Red = Green = Blue = pgm_read_byte(&LEDSteps[LEDCurrentStep]);
          colorFill(Color(Red, Green, Blue));
          if (RC5.FreshData)
            break;
          delay(100);
        }
        if (LEDCurrentStep >= LED_STEP_ARRAY_SIZE)
          LEDCurrentStep = 32;
        break;
      case DOWN_ARROW:
        Status.eBefehl = RC5.Befehl;
        Serial.println(F("Runter"));
        for (; LEDCurrentStep; LEDCurrentStep--) {
          Red = Green = Blue = pgm_read_byte(&LEDSteps[LEDCurrentStep]);
          colorFill(Color(Red, Green, Blue));
          if (RC5.FreshData)
            break;
          delay(100);
        }
        if (!LEDCurrentStep) { //Damit bei Null auch aus ist...
          Red = Green = Blue = pgm_read_byte(&LEDSteps[LEDCurrentStep]);
          colorFill(Color(Red, Green, Blue));
        }
        break;
      case RIGHT_ARROW:
        Serial.println(F("Rechts"));
        break;
      case LEFT_ARROW:
        Serial.println(F("Links"));
        break;
      case SELECT_BUTTON:
        Status.eBefehl = RC5.Befehl;
        Serial.println(F("O.k."));
        break;
      case BUTTON_0:
        Status.eBefehl = RC5.Befehl;
        Serial.println(F("Taste 0"));
        rainbowCycle(WAIT);
        break;
      case BUTTON_1:
        Status.eBefehl = RC5.Befehl;
        Serial.println(F("Taste 1"));
        colorFill(Color(255, 0, 0));
        break;
      case BUTTON_2:
        Status.eBefehl = RC5.Befehl;
        Serial.println(F("Taste 2"));
        colorFill(Color(0, 255, 0));
        break;
      case BUTTON_3:
        Status.eBefehl = RC5.Befehl;
        Serial.println(F("Taste 3"));
        colorFill(Color(0, 0, 255));
        break;
      case BUTTON_4:
        Status.eBefehl = RC5.Befehl;
        Serial.println(F("Taste 4"));
        rainbow(WAIT);
        break;
      case BUTTON_5:
        Status.eBefehl = RC5.Befehl;
        Serial.println(F("Taste 5"));
        rainbowCycle(WAIT);
        break;
      case BUTTON_6:  //Detail-Mischfarben rotieren
        Status.eBefehl = RC5.Befehl;
        Serial.println(F("Taste 6"));
        randomFill();
        rotateStrip(1000);
        break;
      case BUTTON_7:  //zufällige grundfarben rotieren
        Status.eBefehl = RC5.Befehl;
        Serial.println(F("Taste 7"));
        randomFillRGB();
        rotateStrip(1000);
        break;
      case BUTTON_8:  //Punkt in weißem Strip laufen lassen
        Status.eBefehl = RC5.Befehl;
        Serial.println(F("Taste 8"));
        colorFill(0x7f7f7f);
        PickRandomBase();
        DotPosition = random(1, STRIPLEN);
        if(DotPosition >= (STRIPLEN - 10)) {  //Überlauf des wandernden Punkt über Striplen hinaus vermeiden
          DotPosition -= 10;
          }
        for (uint8_t i = 0; i < 5; i++) {
          LEDStrip.setPixelColor(DotPosition++, Color(Red, Green, Blue));
          }
        LEDStrip.show();
        rotateStrip(200);
        break;
      case BUTTON_9:
        Status.eBefehl = RC5.Befehl;
        Serial.println(F("Taste 9"));
        break;
      case BUTTON_STAR:
        Serial.println(F("Stern"));
        Serial.println(F("Sichere Daten..."));
        Status.eBefehl = Status.eBefehl;
        Status.eRed = Red;
        Status.eGreen = Green;
        Status.eBlue = Blue;
        EEPROM.put(0, Status);
        colorFillSlow(0, 50);
        break;
      case BUTTON_HASH:
        Serial.println(F("Hash"));
        Serial.println(F("Daten wiederherstellen...."));
        EEPROM.get(0, Status);
        RC5.Befehl = Status.eBefehl;
        Red = Status.eRed;
        Green = Status.eGreen;
        Blue = Status.eBlue;
        if (RC5.Befehl == SELECT_BUTTON)
          colorFillSlow(Color(Red, Green, Blue), 20);
        RC5.FreshData = 1;
        break;
      default:
        Serial.println(F("Unbekannt"));
        break;
    }
  }
}


//Pins
#define IR_PIN PIND
#define IR_IN PD7
#define IR_TEST (IR_PIN &(1 << IR_IN))
#define IR_SPIEGEL_ON (PORTB |= (1 << PB5))
#define IR_SPIEGEL_OFF (PORTB &= ~(1 << PB5))

//Protokollzeiten
//1125us Null
#define PERIOD_LOW_MIN	350//200
#define PERIOD_LOW_MAX 500 //300
//2250us Eins
#define PERIOD_HIGH_MIN	500 //540
#define PERIOD_HIGH_MAX 650
//13500us Start
#define START_MIN 3000
#define START_MAX 4000
//560us Normalpuls
#define PULS_MIN 300 //140
#define PULS_MAX 450 //180

//Timer2 ein und aus
#define TIMER2_OFF (TCCR2B = 0)
#define TIMER2_ON (TCCR2B = (1 << CS22))

unsigned char Timer0Stop = 0;
uint16_t PulsWidth = 0, PulsPeriod = 0;
uint16_t LastPulsWidth = 0;
volatile unsigned char FreshPulsData = 0, NECValid = 0, StopPuls = 0, PulsCount = 0;
volatile uint32_t NECInput = 0, NECData = 0;

void NEC_Init(void)
{
  //Ports rein und raus
  //       IR Pull-Up
  PORTD = (1 << PD7);	//Arduino Digital 7

  //PCINT23 (Arduino D7, ATMega PD7) freischalten
  PCMSK2 = (1 << PCINT23);
  PCICR = (1 << PCIE2);

  //Timer2
  TCCR2B = (1 << CS22);
  TIMSK2 = (1 << TOIE2);
}

ISR(TIMER2_OVF_vect)	//Laufzeit ca. 4µs bei 16MHz
{
  if ((PulsWidth < 10000) && !StopPuls)
    PulsWidth += 256;
  if (PulsPeriod < 10000)
    PulsPeriod += 256;
  else	//Zu lange Pause, Timer0 anhalten und Status anzeigen
  {
    Timer0Stop = 1;
    TIMER2_OFF;  //Timer2 stoppen
  }
}

ISR(PCINT2_vect)	//Laufzeit ca. 10µs bei 8MHz
{
  if (IR_TEST)	//fallende Flanke IR-Signal
  {
    IR_SPIEGEL_OFF;
    PulsWidth += (uint16_t)TCNT2;
    StopPuls = 1;
    if ((PulsCount >= 32) && ((PulsWidth > PULS_MIN) && (PulsWidth < PULS_MAX)))	//Schlusspuls
    {
      NECValid = 1;	//gültige daten Anzeigen
      NECData = NECInput;	//Daten übergeben
      RC5.Adresse = (uint8_t)((NECInput >> 24) & 0xFF);
      RC5.Befehl = (uint8_t)((NECInput >> 8) & 0xFF);
      LastPulsWidth = PulsWidth;
      RC5.FreshData = 1;
      NECInput = 0;	//nächstes Wort vorbereiten
      PulsCount = 0;	//Pulszähler Zurücksetzen
    }
  }
  else   //steigende Flanke IR-Signal, Bit.Beginn
  {
    IR_SPIEGEL_ON;
    if (Timer0Stop)	//hier fängt ein neues NEC-Signal an
    {
      PulsCount = 0;	//alles initialisieren
      PulsPeriod = 0;
      PulsWidth = 0;
      NECInput = 0;
      TCNT2 = 0;
      Timer0Stop = 0;
      TIMER2_ON;  //Timer2 starten
    }
    else
    {
      PulsPeriod += TCNT2;	//kompletter Wert der letzten Periode
      if ((PulsPeriod > START_MIN) && (PulsPeriod < START_MAX))	//Start Pulsfolge NEC
      {
        PulsCount = 0;
        NECInput = 0;
      }
      else	//eine normale Periode
      {
        if ((PulsPeriod >= PERIOD_HIGH_MIN) && (PulsPeriod <= PERIOD_HIGH_MAX))	//NEC Eins
        {
          NECInput |= ((uint32_t)1 << (31 - PulsCount));
          PulsCount++;
        }
        else if ((PulsPeriod >= PERIOD_LOW_MIN) && (PulsPeriod <= PERIOD_LOW_MAX))	//NEC Null
          PulsCount++;
        else	//Fehler
        {
          PulsCount = 0;
          NECInput = 0;
        }
      }

    }
    TCNT2 = 0;
    StopPuls = 0;
    PulsWidth = 0;
    PulsPeriod = 0;
  }
}


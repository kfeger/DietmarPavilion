/* Gültigkeitsprüfung */
int Check_EE(void) {
  if((EEPROM.read(EECHK0) != 0x55) || (EEPROM.read(EECHK1) != 0xaa)) {
    // EEPROM nicht gültig, neu initialisieren
    // Auf 05:30, Aus 22:00, Tierazrt aus
    EEPROM.write(EECHK0, 0x55);
    EEPROM.write(EECHK1, 0xaa);
    EEPROM.write(EERED, 64);
    EEPROM.write(EEGREEN, 64);
    EEPROM.write(EEBLUE, 64);
    EEPROM.write(EEBRIGHT, 64);
    EEPROM.commit();
    Serial.println("EEPROM initialisiert");
    return(1);
  }
  else {
    Serial.println("EEPROM o.k.");
    return(0);
  }
}

/* Farben aus EEPROM holen */
void SetFromEE(void) {
  rot = EEPROM.read(EERED);
  gruen = EEPROM.read(EEGREEN);
  blau = EEPROM.read(EEBLUE);
  hell = EEPROM.read(EEBRIGHT);
}

void SaveToEE (void) {
    EEPROM.write(EERED, rot);
    EEPROM.write(EEGREEN, gruen);
    EEPROM.write(EEBLUE, blau);
    EEPROM.write(EEBRIGHT, hell);
    EEPROM.commit();  
}

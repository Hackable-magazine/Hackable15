#include <Wire.h> 
#include <extEEPROM.h>
#include <DS3232RTC.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SI1145.h>
#include <Time.h> 

// Notre format de données d'enregistrement
struct MesInfo {
  time_t horo;
  float temp;
  float hygro;
  float pression;
  uint16_t visible;
  uint16_t ir;
  uint16_t uv;
};

// stockage du temps
// c'est un unsigned long dans Time/TimeLib.h
time_t myTime;

// Variable pour nos données
MesInfo mesdata;

// Atmel 24C256 EEPROM
extEEPROM eep(kbits_256, 1, 64);

// Catpeur temp/hygro/pression
Adafruit_BME280 bme;

// Capteur lumière/IR/UV
Adafruit_SI1145 lum = Adafruit_SI1145();

// gestion des intervalles de mesure
unsigned long previousMillis = 0;

// Affichage d'une valeur sur deux chiffres
void print2digits(int val, Stream &sortie) {
  // si <10 alors on préfixe d'un 0
  sortie.print(val < 10 ? "0" : "");
  sortie.print(val, DEC);
}

// Enregistrement d'un relevé
void enreg() {
  // code retour de l'écriture
  int ret = 0;
  
  // lecture du compteur
  int compteur = 0;
  eep.read(0, (byte *) &compteur, sizeof(compteur));

  // remplissage de notre structure
  mesdata.horo=RTC.get();
  mesdata.temp=bme.readTemperature();
  mesdata.hygro=bme.readHumidity();
  mesdata.pression=bme.readPressure()/100.0F;
  mesdata.visible=lum.readVisible();
  mesdata.ir=lum.readIR();
  mesdata.uv=lum.readUV();

  // écriture en EEPROM
  ret = eep.write(sizeof(compteur)+sizeof(mesdata)*compteur, (byte *) &mesdata, sizeof(mesdata));

  // Une erreur ?
  if(ret != 0)
    return;

  // incrémentation du compteur
  compteur++;
  // mise à jour du compteur
  eep.write(0, (byte *) &compteur, sizeof(compteur));
}

// Affichage mesures temp réel
void mesure() {
  mesdata.horo=RTC.get();
  mesdata.temp=bme.readTemperature();
  mesdata.hygro=bme.readHumidity();
  mesdata.pression=bme.readPressure()/100.0F;
  mesdata.visible=lum.readVisible();
  mesdata.ir=lum.readIR();
  mesdata.uv=lum.readUV();

  // Affichage
  print2digits(day(mesdata.horo), Serial);
  Serial.print("/");
  print2digits(month(mesdata.horo), Serial);
  Serial.print("/");
  print2digits(year(mesdata.horo), Serial);
  Serial.print(" ");
  print2digits(hour(mesdata.horo), Serial);
  Serial.print(":");
  print2digits(minute(mesdata.horo), Serial);
  Serial.print(":");
  print2digits(second(mesdata.horo), Serial);
  Serial.print("   temp:");
  Serial.print(mesdata.temp);
  Serial.print("   hygro:");
  Serial.print(mesdata.hygro);
  Serial.print("   pression:");
  Serial.print(mesdata.pression);
  Serial.print("   Visible:");
  Serial.print(mesdata.visible);
  Serial.print("   IR:");
  Serial.print(mesdata.ir);
  Serial.print("   UV:");
  Serial.print(mesdata.uv);
  Serial.println("");
}

// fonction d'affichage des données
void dump() {
  int compteur;

  // lecture du compteur à la position 0 de l'EEPROM
  eep.read(0, (byte *) &compteur, sizeof(compteur));

  if(!compteur) {
    Serial.println("Pas de data");
    return;
  }

  // On boucle sur les enregistrements
  for(int i=0; i<compteur; i++) {
    eep.read(sizeof(compteur)+i*sizeof(mesdata), (byte *) &mesdata, sizeof(mesdata));

    // Affichage
    print2digits(day(mesdata.horo), Serial);
    Serial.print("/");
    print2digits(month(mesdata.horo), Serial);
    Serial.print("/");
    print2digits(year(mesdata.horo), Serial);
    Serial.print(" ");
    print2digits(hour(mesdata.horo), Serial);
    Serial.print(":");
    print2digits(minute(mesdata.horo), Serial);
    Serial.print(":");
    print2digits(second(mesdata.horo), Serial);
    Serial.print(";");
    Serial.print(mesdata.temp);
    Serial.print(";");
    Serial.print(mesdata.hygro);
    Serial.print(";");
    Serial.print(mesdata.pression);
    Serial.print(";");
    Serial.print(mesdata.visible);
    Serial.print(";");
    Serial.print(mesdata.ir);
    Serial.print(";");
    Serial.print(mesdata.uv);
    Serial.println("");
  }
}

// Affichage de la valeur du compteur
// C'est le numéro du dernier enregistrement en mémoire
void printcompteur() {
  int compteur;
  eep.read(0, (byte *) &compteur, sizeof(compteur));
  Serial.print(F("Valeur actuelle du compteur: "));
  Serial.println(compteur);
}

// Effacement de l'EEROM
void reinit() {
  int compteur;
  eep.read(0, (byte *) &compteur, sizeof(compteur));
  Serial.print(F("Valeur actuelle du compteur: "));
  Serial.println(compteur);

  // Uniquement si broche 2 à la masse : cavalier en place
  if(!digitalRead(2)) {
    Serial.println(F("Effacement compteur"));
    compteur=0;
    eep.write(0, (byte *) &compteur, sizeof(compteur));
    eep.read(0, (byte *) &compteur, sizeof(compteur));
    Serial.print("Valeur actuelle du compteur: ");
    Serial.println(compteur);
  } else {
    Serial.println(F("Effacement interdit ! Mauvais mode: mesures en cours"));
    Serial.println(F("Placez le cavalier pour autoriser l'effacement"));
  }
}

// configuration
void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  // Cavalier
  pinMode(2, INPUT_PULLUP); 

  // initialisation capteur BME280
  if (!bme.begin(0x76)){
    Serial.print(F("BME280 erreur"));
    while (1);
  }
  Serial.println(F("BME280 ok"));

  // initialisation capteur SI1145
  if (!lum.begin()){
    Serial.print(F("SI1145 erreur"));
    while (1);
  }
  Serial.println(F("SI1145 ok"));

  // initialisation gestion EEPROM
  if (eep.begin(twiClock400kHz)) {
    Serial.print(F("extEEPROM erreur"));
    while (1);
  }
  Serial.println(F("EEPROM ok"));

  Serial.println(F("OK"));
}

void loop() {
  unsigned long currentMillis = millis();
  String commande;

  // Gestion des intervalle sans delay()
  if (currentMillis - previousMillis >= 60000) {
    previousMillis = currentMillis;
    // Cavalier ?
    if(digitalRead(2))
      enreg();
    else
      Serial.println(F("Cavalier en place: pas d'enregistrement"));
  }

  // récupération d'une chaîne via le moniteur série
  while (Serial.available() > 0) {
    commande = Serial.readStringUntil('\n');
  }

  // Analyse des commandes
  // affichage des données
  if(commande == "dump")
    dump();
  // affichage du compteur
  if(commande == "compteur")
    printcompteur();
  // Effacement EEPROM
  if(commande == "reinit")
    reinit();
  if(commande == "mesure")
    mesure();
}

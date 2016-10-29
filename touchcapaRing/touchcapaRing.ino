#include <WS2812FX.h>
#include <CapacitiveSensor.h>
#include <EEPROM.h>
#include <avr/wdt.h>

#define NBRLED 24
#define LEDP 9

WS2812FX ws2812fx = WS2812FX(NBRLED, LEDP, NEO_GRB + NEO_KHZ800);

CapacitiveSensor lanterne = CapacitiveSensor(4,2);

unsigned long preumsMillis = 0;
int compteur;
uint8_t mode;

void setup(){
   pinMode(LED_BUILTIN, OUTPUT);
   
   lanterne.set_CS_AutocaL_Millis(5000);
   Serial.begin(115200);

   // lecture du dernier mode en eeprom
   EEPROM.get(0, mode);

   // initialisation leds
   ws2812fx.init();
   ws2812fx.setBrightness(255);
   ws2812fx.setSpeed(500);
   ws2812fx.setColor(255, 0, 0);
   // donnée en eeprom valide ?
   if(mode <  MODE_COUNT)
     ws2812fx.setMode(mode);
   else
     ws2812fx.setMode(FX_MODE_STATIC);
   ws2812fx.start();
}

void loop(){
    long val =  lanterne.capacitiveSensor(15);

    unsigned long currentMillis = millis();

    ws2812fx.service();

    if(compteur) {
      // 2s de passées ?
      if(currentMillis - preumsMillis >= 2000)  {
        // oui, on prend en compte le compteur
        Serial.println(compteur);
        switch(compteur) {
          case 1:
               mode = FX_MODE_STATIC;
               break;
          case 2:
               mode = FX_MODE_FADE;
               break;
          case 3:
               mode = FX_MODE_FIREWORKS;
               break;
          case 4:
               mode = FX_MODE_SPARKLE;
               break;
          case 5:
               mode = FX_MODE_RUNNING_LIGHTS;
               break;
          default:
               // bloqué !
               wdt_enable(WDTO_15MS);
               while(1) {};
        }
        ws2812fx.setMode(mode);
        EEPROM.put(0, mode);
        // réinitialisation
        compteur=0;
      } else {
        // dans la seconde
        if(val > 6000) {
          Serial.println(val);
          digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
          compteur++;
          delay(180); // debounce
        }
      }
    } else {
      // compteur à 0
      if(val > 6000) {
        // premier contact
        Serial.println(val);
        preumsMillis = currentMillis;
        compteur++;
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        delay(180); // debounce
      }
    }
}

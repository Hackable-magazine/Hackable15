#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>

// Déclaration NeoPixel
// NEO_RGB pour la led 8mm
// NEO_GRB pour une vraie WS2812b
Adafruit_NeoPixel ws = Adafruit_NeoPixel(1, 9, NEO_RGB + NEO_KHZ800);

// Déclaration port série logiciel
SoftwareSerial btmodule(10, 11); // RX, TX

void setup() {
  // moniteur série en 115200 bps
  // (utile pour débuger)
  Serial.begin(115200);
  while (!Serial);

  // port logiciel en 19200
  // vitesse configurée avec AT+UART=19200
  btmodule.begin(19200);

  // initialisation NeoPixel
  ws.begin();
  // noir
  ws.setPixelColor(0, ws.Color(0,0,0));
  // affichage
  ws.show();
}

void loop() {
  // Des données à lire ?
  if (btmodule.available()) {
    // Oui, on récupère la commande
    String commande = btmodule.readStringUntil(';');
    // on a 5 octets (sans le ";") ?
    if(commande.length() == 5) {
      // oui, c'est une commande valide
      // le second octet est une commande de couleur ?
      if(commande.charAt(1) == 0x00) {
        // oui, on donne au NeoPixel la couleur demandée
        ws.setPixelColor(0, ws.Color(commande.charAt(2),commande.charAt(3),commande.charAt(4)));
        ws.show();
      }
    }
  }
}


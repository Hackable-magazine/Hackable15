#include <SoftwareSerial.h>


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

  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
}

void loop() {
  // Des données à lire ?
  if (btmodule.available()) {
    // Oui, on récupère la commande jusqu'à LF
    String commande = btmodule.readStringUntil('\n');
    if(commande.length() > 0) {
      // on a une commande
      Serial.print("\"");
      Serial.print(commande);
      Serial.println("\"");
      if(commande == "r1on")
        digitalWrite(9, LOW);
      if(commande == "r1off")
        digitalWrite(9, HIGH);
    }
  }
}


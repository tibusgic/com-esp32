#include <Wire.h>

void setup() {
  // Initialisation du moniteur série à 115200 bauds
  Serial.begin(115200);
  while (!Serial); // Attendre que le port série soit prêt
  
  Serial.println("\n--- Démarrage du Scanner I2C ---");
  
  // Initialisation du bus I2C (SDA = D21, SCL = D22 sur ESP32)
  Wire.begin(); 
}

void loop() {
  byte error, address;
  int nDevices = 0;

  Serial.println("Scan en cours...");

  // On teste toutes les adresses I2C possibles (de 1 à 127)
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Composant I2C trouvé à l'adresse 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    }
    else if (error == 4) {
      Serial.print("Erreur inconnue à l'adresse 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }    
  }
  
  if (nDevices == 0) {
    Serial.println("Aucun composant I2C trouvé. Vérifie tes câbles !");
  } else {
    Serial.println("Scan terminé.\n");
  }
  
  // Attendre 5 secondes avant le prochain scan
  delay(5000); 
}
#include <ESP32Servo.h>

// Création des objets pour chaque servomoteur
Servo servo1;
Servo servo2;
Servo servo3;

// Définition des broches
const int brocheServo1 = 4;
const int brocheServo2 = 5;
const int brocheServo3 = 18;

void setup() {
  // Initialisation de la communication série pour voir les résultats sur l'ordinateur
  Serial.begin(115200);
  
  // Attachement des servos aux broches
  servo1.attach(brocheServo1);
  servo2.attach(brocheServo2);
  servo3.attach(brocheServo3);
  

  delay(1000); // On laisse 1 seconde aux moteurs pour atteindre leur position
}

void loop() {
  // Récupération de la dernière position commandée (en degrés)
  int position1 = servo1.read();
  int position2 = servo2.read();
  int position3 = servo3.read();
  
  // Affichage dans le moniteur série
  Serial.print("Position Servo (Broche 4) : ");
  Serial.print(position1);
  Serial.print("° | ");
  
  Serial.print("Servo (Broche 5) : ");
  Serial.print(position2);
  Serial.print("° | ");
  
  Serial.print("Servo (Broche 18) : ");
  Serial.print(position3);
  Serial.println("°");
  
  // Pause de 2 secondes avant la prochaine lecture
  delay(1000);
}
#include <Arduino_JSON.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

struct __attribute__((packed)) MyData {
  float bme280[3];
  int ultrasonic1;
  int ultrasonic2;
  int ultrasonic3;
  
  int lidar;

  int power;

  int motor1;
  int motor2;

  int servo1;
  int servo2;
  int servo3;
  int servo4;
};
const uint8_t TFLUNA_I2C_ADDR = 0x10;

const int sensorD1Pin = 34;

int updateData(String jsonStr);
int send_data(MyData data);
int getDistance();
int getLidarDistance();
int getBMEValues();

MyData data;

Adafruit_BME280 bme;

hw_timer_t *Timer_send_data = NULL; // Timer 
volatile bool flag_send_data = false; // Flag levé par l'ISR

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  Serial.println("Setup...");
  Wire.begin(); // Initialise le bus I2C (SDA = D21, SCL = D22)

  // Initialisation du BME280 à l'adresse 0x76
  if (!bme.begin(0x76)) {
    Serial.println("Erreur : BME280 introuvable à l'adresse 0x76 !");
  } else {
    Serial.println("BME280 initialisé avec succès.");
  }


  // Configuration du timer pour envoyer les données à intervalle régulier
  Timer_send_data = timerBegin(1000000);          // 1 MHz
  timerAttachInterrupt(Timer_send_data, []() IRAM_ATTR {
    flag_send_data = true;                          // Lève le flag uniquement (ISR-safe)
  });
  timerAlarm(Timer_send_data, 400000, true, 0);   // 400ms
}

void loop() {

  // déclenché par le timer, exécuté hors ISR
  if (flag_send_data) {
    flag_send_data = false;
    send_data(data);
  }

  // Lecture des commandes reçues depuis la Raspberry Pi
  if (Serial.available() > 0) {
    String messageRecu = Serial.readStringUntil('\n');
    if (messageRecu.startsWith("pi->")) {
      messageRecu = messageRecu.substring(4); // Enlève le préfixe "pi->"
      updateData(messageRecu);
    }
  }
}

int getBMEValues() {
  // Lecture de la température en degrés Celsius
  data.bme280[0] = bme.readTemperature();
  
  // Lecture de l'humidité en pourcentage (%)
  data.bme280[1] = bme.readHumidity();
  
  // Lecture de la pression (divisée par 100 pour passer de Pascals à hPa/millibars)
  data.bme280[2] = bme.readPressure() / 100.0F;

  // Sécurité : Vérifier si les valeurs lues sont valides (NaN = Not a Number)
  if (isnan(data.bme280[0]) || isnan(data.bme280[1]) || isnan(data.bme280[2])) {
    Serial.println("Erreur de lecture du BME280");
    return -1; // Échec
  }

  return 0; // Succès
}

int getLidarDistance() {
  // 1. Indiquer au TF-Luna qu'on veut lire à partir du registre 0x00
  Wire.beginTransmission(TFLUNA_I2C_ADDR);
  Wire.write(0x00); 
  if (Wire.endTransmission() != 0) {
    Serial.println("Erreur I2C : Lidar TF-Luna introuvable !");
    return -1; // Échec de la communication
  }

  // 2. Demander 2 octets au capteur (Distance Low et Distance High)
  Wire.requestFrom(TFLUNA_I2C_ADDR, (uint8_t)2);
  
  if (Wire.available() >= 2) {
    uint8_t distL = Wire.read(); // 1er octet : poids faible (Low)
    uint8_t distH = Wire.read(); // 2e octet : poids fort (High)
    
    // 3. Assembler les deux octets pour obtenir la distance en cm
    uint16_t distance_cm = (distH << 8) | distL;
    
    // 4. Mettre à jour ta structure de données
    data.lidar = (float)distance_cm;
    
    return 0; // Succès
  }
  
  return -1; // Échec de la lecture des données
}

int getDistance(){
  uint32_t voltage_mV = analogReadMilliVolts(sensorD1Pin);
  float distance_cm = (voltage_mV * 520.0) / 3300.0;
  data.ultrasonic1 = (int) distance_cm;

  return 0;
}

int updateData(String jsonStr) {
  JSONVar doc = JSON.parse(jsonStr);

  if (JSON.typeof(doc) == "undefined") {
    Serial.println("Erreur JSON : parsing failed");
    return -1;
  }

  data.motor1 = (int)    doc["motor1"];
  data.motor2 = (int)    doc["motor2"];
  data.servo1 = (int)    doc["servo1"];
  data.servo2 = (int)    doc["servo2"];
  data.servo3 = (int)    doc["servo3"];
  data.servo4 = (int)    doc["servo4"];

  return 0;
}

int send_data(MyData data) {
  getDistance();
  getLidarDistance();
  getBMEValues();
  JSONVar doc;
  doc["lidar"]       = data.lidar;
  doc["ultrasonic1"] = data.ultrasonic1;
  doc["ultrasonic2"] = data.ultrasonic2;
  doc["ultrasonic3"] = data.ultrasonic3;

  JSONVar bmeArray;
  bmeArray[0] = data.bme280[0]; // Temp
  bmeArray[1] = data.bme280[1]; // Hum
  bmeArray[2] = data.bme280[2]; // Pres

  doc["bme280"] = bmeArray;

  doc["power"] = data.power;

  String jsonStr = JSON.stringify(doc);

  Serial.print("esp->");
  Serial.println(jsonStr);
  
  return 0;
}

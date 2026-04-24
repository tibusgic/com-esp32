#include <Arduino_JSON.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>

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
const int sensorD2Pin = 35;
const int sensorD3Pin = 32;

// MDD10A motor driver pins
const int PWM1_PIN = 25;
const int DIR1_PIN = 33;
const int PWM2_PIN = 27;
const int DIR2_PIN = 26;

//batterie voltage pin
const int BATTERY_VOLTAGE_PIN = 15;

int updateData(String jsonStr);
int send_data(MyData data);
int getDistance();
int getLidarDistance();
int getBMEValues();
int getServosAngles();
int setServosAngles();
int setMotors();
void printServo();
int printLCD(String l1, String l2, int power);
void readBatteryVoltage();

MyData data;

Adafruit_BME280 bme;
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

int brocheServo1 = 4;
int brocheServo2 = 5;
int brocheServo3 = 18;
int brocheServo4 = 19;

LiquidCrystal_I2C lcd(0x27, 20, 4);
String line1 = "WALL-E Ready!";
String line2 = "";

hw_timer_t *Timer_send_data = NULL; // Timer 
volatile bool flag_send_data = false; // Flag levé par l'ISR

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  Serial.println("Setup...");
  Wire.begin(); //I2C (SDA = D21, SCL = D22)

  // BME280 à l'adresse 0x76
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


 // Attachement du servo 1
  if (servo1.attach(brocheServo1, 500, 2400) == -1) {
    Serial.println("ERREUR : Impossible d'attacher le Servo 1 sur D4");
  } else {
    servo1.write(90);
    Serial.println("Servo 1 attaché avec succès !");
  }

  // Attachement du servo 2
  if (servo2.attach(brocheServo2, 500, 2400) == -1) {
    Serial.println("ERREUR : Impossible d'attacher le Servo 2 sur D5 (Vérifie la version de ESP32Servo !)");
  } else {
    servo2.write(90);
    Serial.println("Servo 2 attaché avec succès !");
  }

  // Attachement du servo 3
  if (servo3.attach(brocheServo3, 500, 2400) == -1) {
    Serial.println("ERREUR : Impossible d'attacher le Servo 3 sur D18");
  } else {
    servo3.write(90);
    Serial.println("Servo 3 attaché avec succès !");
  }

  // Attachement du servo 4
  if (servo4.attach(brocheServo4, 500, 2400) == -1) {
    Serial.println("ERREUR : Impossible d'attacher le Servo 4 sur D19");
  } else {
    servo4.write(90);
    Serial.println("Servo 4 attaché avec succès !");
  }

  Serial.println("Lecture des servos");
  data.servo1 = 60;
  data.servo2 = 60;
  data.servo3 = 0;
  data.servo4 = 60;

  setServosAngles();
  getServosAngles();

  // Initialisation des moteurs
  ledcAttach(PWM1_PIN, 1000, 8);
  ledcAttach(PWM2_PIN, 1000, 8);
  pinMode(DIR1_PIN, OUTPUT);
  pinMode(DIR2_PIN, OUTPUT);
  data.motor1 = 0;
  data.motor2 = 0;
  setMotors();
  Serial.println("Moteurs initialisés.");

  // Initialisation de l'écran LCD
  lcd.init();
  lcd.backlight();
  printLCD(line1, line2, data.power);

  //attach du pin de la batterie
  pinMode(BATTERY_VOLTAGE_PIN, INPUT);
  
}

void loop() {

  // déclenché par le timer, exécuté hors ISR
  if (flag_send_data) {
    flag_send_data = false;
    send_data(data);
    printServo();
  }

  // Lecture des commandes reçues depuis la Raspberry Pi
  if (Serial.available() > 0) {
    String messageRecu = Serial.readStringUntil('\n');
    if (messageRecu.startsWith("pi->")) {
      messageRecu = messageRecu.substring(4); // Enlève le préfixe "pi->"
      updateData(messageRecu);
      setServosAngles();
      setMotors();
    }
    else if (messageRecu.startsWith("pimsg->")) {
      messageRecu = messageRecu.substring(7); // Enlève le préfixe "pimsg->"
      line1 = messageRecu.substring(0, messageRecu.indexOf('|'));
      line2 = messageRecu.substring(messageRecu.indexOf('|') + 1);
      printLCD(line1, line2, data.power);
    }
  }
}

void printServo(){
Serial.printf("Servo1: %d, Servo2: %d, Servo3: %d\n", data.servo1, data.servo2, data.servo3);
}

int getServosAngles(){
  data.servo1 = servo1.read();
  data.servo2 = servo2.read();
  data.servo3 = servo3.read();
  data.servo4 = servo4.read();
  return 0;
}
int setServosAngles(){
  servo1.write(data.servo1);
  servo2.write(data.servo2);
  servo3.write(data.servo3);
  servo4.write(data.servo4);
  return 0;
}

int getBMEValues() {
  data.bme280[0] = bme.readTemperature();
  
  data.bme280[1] = bme.readHumidity();
  
  data.bme280[2] = bme.readPressure() / 100.0F;

  // Sécurité : Vérifier si les valeurs lues sont valides (NaN = Not a Number)
  if (isnan(data.bme280[0]) || isnan(data.bme280[1]) || isnan(data.bme280[2])) {
    Serial.println("Erreur de lecture du BME280");
    return -1; 
  }

  return 0; 
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
    
    return 0; 
  }
  
  return -1; 
}

int getDistance(){
  uint32_t voltage_mV = analogReadMilliVolts(sensorD1Pin);
  float distance_cm = (voltage_mV * 520.0) / 3300.0;
  data.ultrasonic1 = (int) distance_cm;

  voltage_mV = analogReadMilliVolts(sensorD2Pin);
  distance_cm = (voltage_mV * 520.0) / 3300.0;
  data.ultrasonic2 = (int) distance_cm;

  voltage_mV = analogReadMilliVolts(sensorD3Pin);
  distance_cm = (voltage_mV * 520.0) / 3300.0;
  data.ultrasonic3 = (int) distance_cm;

  return 0;
}

int setMotors() {
  if (data.motor1 >= 0) {
    digitalWrite(DIR1_PIN, HIGH);
    ledcWrite(PWM1_PIN, data.motor1);
  } else {
    digitalWrite(DIR1_PIN, LOW);
    ledcWrite(PWM1_PIN, -data.motor1);
  }

  // Motor 2
  if (data.motor2 >= 0) {
    digitalWrite(DIR2_PIN, HIGH);
    ledcWrite(PWM2_PIN, data.motor2);
  } else {
    digitalWrite(DIR2_PIN, LOW);
    ledcWrite(PWM2_PIN, -data.motor2);
  }

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
  readBatteryVoltage();
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

int printLCD(String l1, String l2, int power) {
  String powerStr = String(power) + "%";
  int maxL2 = 20 - (int)powerStr.length();
  if ((int)l2.length() > maxL2) {
    l2 = l2.substring(0, maxL2);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(l1);
  lcd.setCursor(0, 1);
  lcd.print(l2);
  lcd.setCursor(20 - (int)powerStr.length(), 1);
  lcd.print(powerStr);
  return 0;
}


void readBatteryVoltage() {
  uint32_t vOut_mV = analogReadMilliVolts(BATTERY_VOLTAGE_PIN);
  float voltage = (vOut_mV / 1000.0) * 3.96;
  int percentage = (int)((voltage - 10.5) / (12.6 - 10.5) * 100);
  if (percentage > 100) percentage = 100;
  if (percentage < 0)   percentage = 0;
  data.power = (int)((percentage + data.power*10)/11);
}
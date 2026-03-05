#include <Arduino_JSON.h>

struct __attribute__((packed)) MyData {
  float bme280[3];
  float ultrasonic1;
  float ultrasonic2;
  float ultrasonic3;
  
  float lidar;

  int power;

  int motor1;
  int motor2;

  int servo1;
  int servo2;
  int servo3;
  int servo4;
};

int updateData(String jsonStr);
int send_data(MyData data);

MyData data;

hw_timer_t *Timer_send_data = NULL; // Timer 
volatile bool flag_send_data = false; // Flag levé par l'ISR

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  Serial.println("Setup...");


  // Configuration du timer pour envoyer les données à intervalle régulier
  Timer_send_data = timerBegin(1000000);          // 1 MHz
  timerAttachInterrupt(Timer_send_data, []() IRAM_ATTR {
    flag_send_data = true;                          // Lève le flag uniquement (ISR-safe)
  });
  timerAlarm(Timer_send_data, 1000000, true, 0);   // 1s
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

import json

class dataWalle:
    """
    Classe pour gérer les données reçues de l'ESP32.

    Attributs:
        servo1 (int): Valeur du servo 1.
        servo2 (int): Valeur du servo 2.
        servo3 (int): Valeur du servo 3.
        servo4 (int): Valeur du servo 4.
        motor1 (int): Valeur du moteur 1.
        motor2 (int): Valeur du moteur 2.
        lidar (int): Valeur du lidar.
        ultrasonic1 (int): Valeur de l'ultrason 1.
        ultrasonic2 (int): Valeur de l'ultrason 2.
        ultrasonic3 (int): Valeur de l'ultrason 3.
        bme280 (list): Liste contenant les valeurs [temperature, pression, humidité] (float) du capteur BME280.
        power (float): Valeur de la batterie.

    Méthodes:
        __init__(self): Initialise l'objet dataWalle.
        update_data(self, servo1, servo2, servo3, servo4, motor1, motor2, lidar, ultrasonic1, ultrasonic2, ultrasonic3, bme280, power): Met à jour les données reçues avec des nouvelles valeur.
        update_data_from_message(self, message): Met à jour les données reçues à partir d'un message JSON.
        get_all_data(self): Récupère les données reçues sous forme de dictionnaire avec les valeurs des attributs.
        get_action_data(self): Récupère les données d'action (servo et moteur) sous forme de dictionnaire avec les valeurs des attributs.
        to_json(self, data): Récupère les données reçues sous forme de json avec les valeurs des attributs.
    """
    def __init__(self):
        """
        Initialise l'objet dataWalle.
        """
        self.servo1 = 0             # Valeur du servo 1
        self.servo2 = 0             # Valeur du servo 2
        self.servo3 = 0             # Valeur du servo 3
        self.servo4 = 0             # Valeur du servo 4

        self.motor1 = 0           # Valeur du moteur 1
        self.motor2 = 0           # Valeur du moteur 2


        self.lidar = 0            # Valeur du lidar
        self.ultrasonic1 = 0      # Valeur de l'ultrason 1
        self.ultrasonic2 = 0      # Valeur de l'ultrason 2
        self.ultrasonic3 = 0      # Valeur de l'ultrason 3

        self.bme280 = [0.0,0.0,0.0]       # [temperature, pression, humidité]

        self.power = 0            # Valeur de la batterie





    def update_data(self, servo1=None, servo2=None, servo3=None, servo4=None, motor1=None, motor2=None, lidar=None, ultrasonic1=None, ultrasonic2=None, ultrasonic3=None, bme280=None, power=None):
        """
        Met à jour les données reçues avec des nouvelles valeur.

        Args:
            servo1 (int): Valeur du servo 1.
            servo2 (int): Valeur du servo 2.
            servo3 (int): Valeur du servo 3.
            servo4 (int): Valeur du servo 4.
            motor1 (int): Valeur du moteur 1.
            motor2 (int): Valeur du moteur 2.
            lidar (float): Valeur du lidar.
            ultrasonic1 (int): Valeur de l'ultrason 1.
            ultrasonic2 (int): Valeur de l'ultrason 2.
            ultrasonic3 (int): Valeur de l'ultrason 3.
            bme280 (list): Liste contenant les valeurs [temperature, pression, humidité] (float) du capteur BME280.
            power (int): Valeur de la batterie.

        """
        self.servo1 = servo1
        self.servo2 = servo2
        self.servo3 = servo3
        self.servo4 = servo4
        self.motor1 = motor1
        self.motor2 = motor2
        self.lidar = lidar
        self.ultrasonic1 = ultrasonic1
        self.ultrasonic2 = ultrasonic2
        self.ultrasonic3 = ultrasonic3
        self.bme280 = bme280
        self.power = power

    def update_data_from_message(self, message):
        """
        Met à jour les données reçues à partir d'un message JSON.

        Args:
            message (str): Message JSON contenant les données à mettre à jour.
        """
        try:
            data = json.loads(message)
            self.update_data(
                servo1=self.servo1,
                servo2=self.servo2,
                servo3=self.servo3,
                servo4=self.servo4,
                motor1=self.motor1,
                motor2=self.motor2,
                lidar=data.get("lidar", self.lidar),
                ultrasonic1=data.get("ultrasonic1", self.ultrasonic1),
                ultrasonic2=data.get("ultrasonic2", self.ultrasonic2),
                ultrasonic3=data.get("ultrasonic3", self.ultrasonic3),
                bme280=data.get("bme280", self.bme280),
                power=data.get("power", self.power)
            )
        except json.JSONDecodeError as e:
            print(f"Erreur de décodage JSON : {e}")

    def get_all_data(self):
        """
        Récupère les données reçues sous forme de dictionnaire avec les valeurs des attributs.

        Returns:
            dict: Un dictionnaire contenant les données reçues.
        """
        return {
            "servo1": self.servo1,
            "servo2": self.servo2,
            "servo3": self.servo3,
            "servo4": self.servo4,
            "motor1": self.motor1,
            "motor2": self.motor2,
            "lidar": self.lidar,
            "ultrasonic1": self.ultrasonic1,
            "ultrasonic2": self.ultrasonic2,
            "ultrasonic3": self.ultrasonic3,
            "bme280": self.bme280,
            "power": self.power
        }
    
    def get_action_data(self):
        """
        Récupère les données d'action (servo et moteur) sous forme de dictionnaire avec les valeurs des attributs.

        Returns:
            dict: Un dictionnaire contenant les données d'action reçues.
        """
        return {
            "servo1": self.servo1,
            "servo2": self.servo2,
            "servo3": self.servo3,
            "servo4": self.servo4,
            "motor1": self.motor1,
            "motor2": self.motor2
        }

    def to_json(self, data):
        """
        Récupère les données reçues sous forme de json avec les valeurs des attributs.
        Args:
            data (dict): Les données à convertir en JSON.
        Returns:
            str: Une chaîne JSON contenant les données reçues.
        """
        return json.dumps(data)
    
    def __str__(self):
        return str(self.get_all_data())
    

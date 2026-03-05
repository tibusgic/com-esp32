import serial
import time
import threading 
from dataWalle import dataWalle

class espWalle:
    """
    Classe pour gérer la communication série avec un ESP32.

    Attributs:
        port_serie (str): Le port série de l'ESP32.
        baudrate (int): La vitesse de communication de l'ESP32.
        esp32 (serial.Serial): L'objet de connexion série avec l'ESP32.
        _running (bool): Flag d'arrêt du thread.

    Méthodes:
        __init__(self, port_serie, baudrate): Initialise l'objet avec les paramètres spécifiés.
        connect(self): Établit la connexion série avec l'ESP32.
        disconnect(self): Ferme la connexion série avec l'ESP32.
        is_connected(self): Vérifie si la connexion série avec l'ESP32 est établie.
        startReadThread(self): Démarre un thread de lecture pour recevoir les données de l'ESP32.
        read_from_esp32(self): Lit les données de l'ESP32 en continu et les affiche.
        send_message(self, message): Envoie un message à l'ESP32 via la connexion série.
        send_action_data(self): Envoie les données stockées dans l'objet dataWalle (seulement les actionneurs) à l'ESP32 via la connexion série.
    """
    def __init__(self, port_serie, baudrate):
        """
        Initialise l'objet espWalle avec les paramètres spécifiés.
        Args:
            port_serie (str): Le port série de l'ESP32.
            baudrate (int): La vitesse de communication de l'ESP32.

        """
        self.port_serie = port_serie            # Port série de l'ESP32
        self.baudrate = baudrate                # Baudrate de communication
        self.esp32 = None                       # Objet de connexion série
        self._running = False                   # Flag d'arrêt du thread
        self._thread = None                     # Thread de lecture
        self._lock = threading.Lock()           # Verrou pour accès thread-safe aux données
        self._callback = None                   # Callback appelé à chaque réception de données

        self.data = dataWalle()       # Objet dataWalle pour stocker les données reçues de l'ESP32

    def connect(self):
        """
        Établit la connexion série avec l'ESP32.
        Raises:
            serial.SerialException: Si le port série ne peut pas être ouvert.
        """
        try:
            print(f"Connexion à l'ESP32 sur {self.port_serie}...")
            self.esp32 = serial.Serial(port=self.port_serie, baudrate=self.baudrate, timeout=1)
            time.sleep(2)
            print("ESP32 prêt à recevoir des données.")
        except serial.SerialException as e:
            print(f"Erreur : Impossible d'ouvrir le port {self.port_serie}. Vérifiez s'il n'est pas utilisé par le moniteur série Arduino.")

    def disconnect(self):
        """
        Ferme la connexion série avec l'ESP32.
        """
        self._running = False
        if self._thread is not None:
            self._thread.join(timeout=2)
        if self.esp32 and self.esp32.is_open:
            self.esp32.close()
            print("Connexion série fermée.")

    def is_connected(self):
        """
        Vérifie si la connexion série avec l'ESP32 est établie
        Returns:
            bool: True si la connexion est établie, False sinon.
        """
        return self.esp32 is not None and self.esp32.is_open

    def startReadThread(self, callback=None):
        """
        Démarre un thread de lecture pour recevoir les données de l'ESP32.
        Args:
            callback (callable): Fonction appelée à chaque réception de données capteurs. Reçoit l'objet dataWalle en argument.
        Raises:
            Exception: Si la connexion série n'est pas établie.
        """
        if self.esp32 is None:
            raise Exception("Connexion série non établie. Veuillez appeler connect() avant de démarrer le thread de lecture.")
        
        self._callback = callback
        self._running = True
        self._thread = threading.Thread(target=self.read_from_esp32, daemon=True)
        self._thread.start()

    def read_from_esp32(self):
        """
        Lit les données de l'ESP32 en continu et les affiche.
        Raises:
            Exception: Si la connexion série n'est pas établie.
        """
        if self.esp32 is None:
            raise Exception("Connexion série non établie. Veuillez appeler connect() avant de lire les données.")
        
        print("Thread de lecture démarré...")
        while self._running:
            try:
                line = self.esp32.readline()
                if line:
                    decoded_data = line.decode('utf-8', errors='ignore').strip()
                    if decoded_data:
                        if decoded_data.startswith("esp->"):
                            with self._lock:
                                self.data.update_data_from_message(decoded_data[5:])
                            if self._callback:
                                self._callback(self.data)
                        elif decoded_data.startswith("Erreur"):
                            print(f"[ESP32] {decoded_data}")
            except Exception as e:
                if self._running:
                    print(f"Erreur de lecture : {e}")
                break
        print("Thread de lecture arrêté.")

    
    def send_message(self, message):
        """
        Envoie un message à l'ESP32 via la connexion série.
        Args:
            message (str): Le message à envoyer.
        Raises:
            Exception: Si la connexion série n'est pas établie.
        """
        if self.esp32 is None:
            raise Exception("Connexion série non établie. Veuillez appeler connect() avant d'envoyer des messages.")
        
        self.esp32.write(("pimsg->"+message + "\n").encode('utf-8'))
        print("pimsg->"+message.strip())

    def send_action_data(self):
        """
        Envoie les données stockées dans l'objet dataWalle (seulement les actionneurs) à l'ESP32 via la connexion série.
        
        Raises:
            Exception: Si la connexion série n'est pas établie.
        """
        if self.esp32 is None:
            raise Exception("Connexion série non établie. Veuillez appeler connect() avant d'envoyer des données.")
        
        data_json = self.data.to_json(self.data.get_action_data())

        self.esp32.write(("pi->"+data_json + "\n").encode('utf-8'))
        print("pi->"+data_json.strip())

    

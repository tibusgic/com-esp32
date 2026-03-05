from espWalle import espWalle
import time
import os

# ──────────────────────────────────────────────
#  Affichage
# ──────────────────────────────────────────────
def clear():
    os.system('cls' if os.name == 'nt' else 'clear')

def display_data(data):
    """Affiche les données capteurs et actionneurs de manière lisible."""
    clear()
    print("╔════════════════════════════════════╗")
    print("║       WALLE  –  Monitoring         ║")
    print("╠════════════════════════════════════╣")
    print(f"║   Batterie     : {str(data.power):<18}║")
    print("╠══════════ Capteurs ════════════════╣")
    print(f"║   Lidar        : {str(data.lidar) + ' cm':<18}║")
    print(f"║   Ultrason 1   : {str(data.ultrasonic1) + ' cm':<18}║")
    print(f"║   Ultrason 2   : {str(data.ultrasonic2) + ' cm':<18}║")
    print(f"║   Ultrason 3   : {str(data.ultrasonic3) + ' cm':<18}║")
    print(f"║   Température  : {str(data.bme280[0]) + ' °C':<18}║")
    print(f"║   Humidité     : {str(data.bme280[1]) + ' %':<18}║")
    print(f"║   Pression     : {str(data.bme280[2]) + ' hPa':<18}║")
    print("╠══════════ Actionneurs ═════════════╣")
    print(f"║   Servo 1      : {str(data.servo1) + '°':<18}║")
    print(f"║   Servo 2      : {str(data.servo2) + '°':<18}║")
    print(f"║   Servo 3      : {str(data.servo3) + '°':<18}║")
    print(f"║   Servo 4      : {str(data.servo4) + '°':<18}║")
    print(f"║   Moteur 1     : {str(data.motor1):<18}║")
    print(f"║   Moteur 2     : {str(data.motor2):<18}║")
    print("╠════════════════════════════════════╣")
    print("║  [Ctrl+C] pour quitter             ║")
    print("╚════════════════════════════════════╝")

# ──────────────────────────────────────────────
#  Main
# ──────────────────────────────────────────────
def main():
    esp = espWalle("COM4", 115200)                      # Objet espWalle avec le port série et le baudrate de l'ESP32
    esp.connect()                                       # Connexion série avec l'ESP32

    if esp.is_connected():
        print("Connexion série établie.")
    else:
        print("Erreur : Connexion série non établie.")
        return

    time.sleep(2)

    esp.startReadThread(callback=display_data)          # Démarre le thread avec affichage à chaque réception

    esp.send_message("Hello ESP32!")                    # Envoie un message à l'ESP32
    esp.data.update_data(servo1=90, servo2=45, servo3=135, servo4=180, motor1=255, motor2=255)
    esp.send_action_data()                              # Envoie les données d'action à l'ESP32

    try:
        while True:
            time.sleep(0.1)                             # Boucle principale : attend les données
    except KeyboardInterrupt:
        print("\nArrêt demandé par l'utilisateur...")

    esp.disconnect()                                    # Ferme la connexion série avec l'ESP32

if __name__ == "__main__":
    main()
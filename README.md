# com-esp32 — Wall-E Serial Communication

Python + Arduino communication system for the Wall-E robot project.  
Handles bidirectional serial communication between a Raspberry Pi (Python) and an ESP32 (Arduino) over USB serial.

---

## Architecture

```
Raspberry Pi (Python)                   ESP32 (Arduino)
─────────────────────────────────────────────────────────
com_esp.py                              sketch_walle.ino
  └── espWalle                ←──────→  loop()
        └── dataWalle                     ├── updateData()
                                          └── send_data()
```

### Protocol

| Direction   | Prefix     | Payload         | Description                        |
|-------------|------------|-----------------|------------------------------------|
| Pi → ESP32  | `pi->`     | JSON string     | Send actuator commands             |
| Pi → ESP32  | `pimsg->`  | plain text      | Send a text message                |
| ESP32 → Pi  | `esp->`    | JSON string     | Send sensor data back              |

All messages are terminated with `\n`.

#### Pi → ESP32 JSON format
```json
{"servo1": 90, "servo2": 45, "servo3": 135, "servo4": 180, "motor1": 255, "motor2": 255}
```

#### ESP32 → Pi JSON format
```json
{"lidar": 42.5, "ultrasonic1": 30.0, "ultrasonic2": 28.0, "ultrasonic3": 31.0, "bme280": [22.4, 55.1, 1013.2], "power": 87}
```

---

## Project Structure

```
com-esp32/
├── com_esp.py              # Entry point — main loop + terminal display
├── espWalle.py             # Serial communication class (connect, send, receive)
├── dataWalle.py            # Data model class (sensors + actuators)
└── sketch_walle/
    └── sketch_walle.ino    # ESP32 Arduino firmware
```

---

## Data Model (`dataWalle`)

| Field        | Type    | Direction    | Description              |
|--------------|---------|--------------|--------------------------|
| `servo1–4`   | float   | Pi → ESP32   | Servo angles (degrees)   |
| `motor1–2`   | float   | Pi → ESP32   | Motor speed              |
| `lidar`      | float   | ESP32 → Pi   | Lidar distance (cm)      |
| `ultrasonic1–3` | float | ESP32 → Pi  | Ultrasonic distances (cm)|
| `bme280`     | list[3] | ESP32 → Pi   | [temp °C, humidity %, pressure hPa] |
| `power`      | float   | ESP32 → Pi   | Battery level            |

---

## Requirements

### Python
- Python 3.x
- `pyserial`

```bash
pip install pyserial
```

### Arduino
- [Arduino_JSON](https://github.com/arduino-libraries/Arduino_JSON) library (install via Arduino Library Manager)

---

## Usage

1. Flash `sketch_walle/sketch_walle.ino` to the ESP32 via Arduino IDE.
2. Edit the port in `com_esp.py` if needed (default: `COM4`):
   ```python
   esp = espWalle("COM4", 115200)
   ```
3. Run the Python script:
   ```bash
   python com_esp.py
   ```

### Terminal display

```
╔════════════════════════════════════╗
║       WALLE  –  Monitoring         ║
╠════════════════════════════════════╣
║   Batterie     : 87                ║
╠══════════ Capteurs ════════════════╣
║   Lidar        : 42.5 cm           ║
║   Ultrason 1   : 30.0 cm           ║
║   Ultrason 2   : 28.0 cm           ║
║   Ultrason 3   : 31.0 cm           ║
║   Température  : 22.4 °C           ║
║   Humidité     : 55.1 %            ║
║   Pression     : 1013.2 hPa        ║
╠══════════ Actionneurs ═════════════╣
║   Servo 1      : 90°               ║
║   Servo 2      : 45°               ║
║   Servo 3      : 135°              ║
║   Servo 4      : 180°              ║
║   Moteur 1     : 255               ║
║   Moteur 2     : 255               ║
╠════════════════════════════════════╣
║  [Ctrl+C] pour quitter             ║
╚════════════════════════════════════╝
```

Press `Ctrl+C` to stop.

---

## ESP32 Firmware overview

- Listens on Serial at `115200` baud
- On `pi->` prefix: parses JSON and updates actuator values (`updateData()`)
- After each received message: sends sensor data back (`send_data()`)
- Uses [Arduino_JSON](https://github.com/arduino-libraries/Arduino_JSON) for JSON parsing/serialization

---

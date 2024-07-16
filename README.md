# teznia
Projekt tężnia 

## Overview
TĘŻNIA project is an automatic mini brine graduation tower. It uses a Raspberry Pi, ESP8266 microcontrollers, DHT22 sensor, OLED display (I2C), pump controlled by relay, and construction of birch twigs.

## Hardware Components
- Water tank 
- Raspberry Pi 
- Power brick for Raspberry Pi
- ESP8266 microcontrollers with Wi-Fi
- DHT22 air temperature and humidity sensors
- Cables and jumpers
- Relay modules
- Pump operating in a salty environment
- Mini OLED screen (128x64 I2C)
- Waterproof boxes for electronics
- Old iPad for web interface

## Software Components
- Raspberry Pi: Flask API
- ESP8266: Sensor data collection, Wi-Fi communication, pump control, OLED display
- Web Interface: Flask-Admin, Bootstrap

## Installation

1. **Set up Raspberry Pi:**
    ```sh
    sudo apt-get update
    sudo apt-get install python3-pip
    pip3 install -r backend/requirements.txt
    ```

2. **Run Flask API:**
    ```sh
    python3 backend/app.py
    ```

3. **Deploy ESP8266 Firmware:**
    - Use Arduino IDE to upload `firmware/executor_firmware/executor_firmware.ino` and `firmware/collector_firmware/collector_firmware.ino` to the respective ESP8266 modules.

## Usage
- Access the web interface through the Raspberry Pi's IP address on port 5000.
- Control the pump via the "Control Pump" section.
- Monitor sensor data and pump logs through the "Dashboard" section.

## License
MIT License
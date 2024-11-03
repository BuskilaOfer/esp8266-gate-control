# ESP8266 Gate Control System

An ESP8266-based gate control system that automates gate operations based on real-time data fetched from a server and allows manual control via a button. The system provides visual feedback through LEDs and logs detailed information via serial communication for debugging and monitoring purposes.

## Table of Contents

- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Setup and Installation](#setup-and-installation)

## Features

- **Automated Gate Control:** Opens the gate when a specific term is found in data fetched from a server.
- **Manual Gate Control:** Allows manual gate opening via a physical button.
- **Visual Feedback:** Uses LEDs to indicate system status, errors, and operations.
- **Logging:** Outputs detailed logs with timestamps over serial communication for debugging.
- **Secure Data Fetching:** Uses HTTPS to securely fetch data from the server.

## Hardware Requirements

- **ESP8266 Microcontroller** (e.g., NodeMCU ESP8266)
- **LEDs:**
  - Green LED (GPIO12 / D6)
  - Blue LED (GPIO13 / D7)
  - Red LED (GPIO15 / D8)
  - Built-in LED (GPIO2 / D4)
- **Gate Control Pins:**
  - Gate Pin 1 (GPIO14 / D5)
  - Gate Pin 2 (GPIO16 / D0)
- **Button:** Connected to GPIO4 (D2) with an internal pull-up resistor
- **WiFi Network:** Access to a 2.4GHz WiFi network

## Software Requirements

- **Arduino IDE** (version 1.8 or higher)
- **ESP8266 Board Package** (installed via Arduino Boards Manager)
- **Arduino Libraries:**
  - `ESP8266WiFi.h`
  - `WiFiClientSecure.h`
  - `ArduinoJson.h` (version compatible with the code)

## Setup and Installation

1. **Clone or Download the Repository:**

   ```bash
   git clone https://github.com/yourusername/esp8266-gate-control.git

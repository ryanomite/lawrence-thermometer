# Lawrence Office Thermometer

ESP8266 (D1 Mini) firmware for monitoring temperature and humidity in the Lawrence office. Reads DHT11 sensor data, publishes to MQTT, and provides both local and public web interfaces.

## Features

- **DHT11 Temperature/Humidity Sensor** on pin D7
- **Temperature in Fahrenheit** with 1 decimal precision
- **WiFi Connectivity** with automatic fallback AP and captive portal
- **MQTT Publishing** to public HiveMQ broker (retained messages)
- **Local Web Interface** for diagnostics
- **Public Web Dashboard** accessible anywhere
- **Smart Publishing** - only publishes on temperature change

## Hardware Setup

### Components
- ESP8266 D1 Mini
- DHT11 Temperature/Humidity Sensor
- USB cable for power/programming

### Wiring
```
DHT11 Sensor → D1 Mini
--------------------
VCC (+)      → 3.3V
DATA         → D7
GND (-)      → GND
```

## Software Setup

### 1. Build and Upload

```bash
# Build the firmware
pio run

# Upload to device (connect D1 Mini via USB)
pio run --target upload

# Monitor serial output
pio device monitor
```

### 2. WiFi Configuration

Create a `credentials.h` file in the project root with your WiFi credentials:

```cpp
#ifndef CREDENTIALS_H
#define CREDENTIALS_H

// WiFi credentials
const char* WIFI_SSID = "your-ssid";
const char* WIFI_PASSWORD = "your-password";

#endif
```

The device will automatically connect to the specified WiFi network on boot.

### 3. Reset WiFi Settings

To reconfigure WiFi, you have two options:
1. Flash the firmware again (erases saved credentials)
2. Hold reset for 10+ seconds (if you implement a reset button)

## Usage

### Local Access

Once connected to WiFi, the device hosts a diagnostic web page:

```
http://<device-ip>/
```

The local IP is displayed in:
- Serial monitor during boot
- MQTT topic `adc-lawrence/ip`

**Local Page Features:**
- Current temperature (°F)
- Humidity percentage
- Device status (WiFi signal, MQTT connection, uptime)
- Auto-refreshes every 30 seconds

### Public Access

The web dashboard is deployed at:
```
https://ryanomite.github.io/lawrence-thermometer/
```

The page:
- Connects to HiveMQ via WebSocket
- Subscribes to `adc-lawrence/temperature`
- Displays real-time temperature
- Shows device IP and last update time

## MQTT Topics

### Publishing
- **Topic**: `adc-lawrence/temperature`
  - **Payload**: Temperature in °F (e.g., `72.5`)
  - **Retained**: Yes (last value persists)
  - **Frequency**: Only on temperature change

- **Topic**: `adc-lawrence/ip`
  - **Payload**: Device's local IP address
  - **Retained**: Yes
  - **Frequency**: Once on boot

### Broker
- **Host**: `broker.hivemq.com`
- **Port**: 1883 (MQTT) / 8884 (WebSocket)
- **Type**: Public, no authentication

## Operation

### Sensor Reading
- Polls DHT11 every **30 seconds**
- Only publishes when temperature changes
- Converts Celsius → Fahrenheit (1 decimal precision)

### LED Indicator
The built-in LED shows status:
- **Blinking**: Normal operation
- **Solid**: WiFi/MQTT connection issue

## Configuration

Edit `src/main.cpp` to customize:

```cpp
// DHT11 pin
#define DHTPIN D7

// Sensor reading interval
const unsigned long READ_INTERVAL = 30000; // 30 seconds

// MQTT broker and topics
const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_topic_temp = "adc-lawrence/temperature";
const char* mqtt_topic_ip = "adc-lawrence/ip";

// Device name
const char* device_name = "Lawrence Office Thermometer";
```

## Troubleshooting

### WiFi Won't Connect
1. Check SSID/password are correct in `credentials.h`
2. Ensure 2.4GHz WiFi (ESP8266 doesn't support 5GHz)
3. Check WiFi signal strength in serial monitor

### No Temperature Reading
1. Verify DHT11 wiring (VCC, GND, Data)
2. Check serial monitor for "Sensor Error"
3. DHT11 requires stable 3.3V power

### MQTT Not Publishing
1. Check internet connectivity
2. Verify `broker.hivemq.com` is accessible
3. Check serial monitor for MQTT status
4. Confirm temperature is actually changing

### Can't Access Local Web Page
1. Find device IP in serial monitor
2. Ensure device and computer are on same network
3. Try `http://` not `https://`

## Project Structure

```
├── src/
│   └── main.cpp              # Main firmware code
├── web/
│   └── index.html            # Public web dashboard
├── include/                   # Header files
├── lib/                       # Custom libraries
├── test/                      # Unit tests
├── platformio.ini             # PlatformIO configuration
├── credentials.h              # WiFi credentials (git ignored)
└── README.md                  # This file
```

## Libraries Used

- **DHT sensor library** (1.4.4) - DHT11/DHT22 sensor support
- **Adafruit Unified Sensor** (1.1.14) - Sensor abstraction layer
- **PubSubClient** (2.8) - MQTT client library

## Memory Usage

- **RAM**: 36.9% (30,256 / 81,920 bytes)
- **Flash**: 28.7% (299,624 / 1,044,464 bytes)

## License

MIT License - Feel free to modify and use as needed.
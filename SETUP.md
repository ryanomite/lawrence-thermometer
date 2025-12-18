# Lawrence Office Thermometer - Setup Guide

## Quick Start Checklist

- [ ] Create `credentials.h` with WiFi credentials
- [ ] Hardware assembled (DHT11 connected to D7)
- [ ] Firmware uploaded via PlatformIO
- [ ] Device connected and publishing to MQTT
- [ ] Public web dashboard accessible

## Initial Setup

### 1. WiFi Credentials

Create a file named `credentials.h` in the project root:

```cpp
#ifndef CREDENTIALS_H
#define CREDENTIALS_H

// WiFi credentials - customize with your network info
const char* WIFI_SSID = "your-ssid";
const char* WIFI_PASSWORD = "your-password";

#endif
```

**Note**: This file is excluded from git (see `.gitignore`), so your credentials stay private.

### 2. Hardware Assembly

Connect DHT11 sensor to D1 Mini:
```
DHT11 VCC  → D1 Mini 3.3V
DHT11 DATA → D1 Mini D7
DHT11 GND  → D1 Mini GND
```

### 3. Upload Firmware

```bash
# In the project directory
pio run --target upload

# Start serial monitor to verify
pio device monitor
```

### 4. Verify Connection

In serial monitor, you should see:
```
WiFi connected successfully!
IP address: 192.168.1.XXX
Connecting to MQTT broker... connected!
Publishing device IP to MQTT: 192.168.1.XXX
```

### 5. Verify MQTT

In serial monitor, you should see:
```
--- Reading DHT11 sensor ---
Temperature: 72.5 °F (22.5 °C), Humidity: 45.0 %
Temperature changed! Publishing to MQTT: 72.5 °F
✓ Published successfully (retained)
```

### 6. Test Local Web Interface

Open browser and navigate to device IP:
```
http://192.168.1.XXX/
```

You should see the diagnostic page with current temperature.

### 7. Access Public Dashboard

Open the public web dashboard from anywhere:
```
https://ryanomite.github.io/lawrence-thermometer/
```

It will display real-time temperature from HiveMQ MQTT broker.

## Testing

### Test DHT11 Sensor
Watch serial monitor for:
```
--- Reading DHT11 sensor ---
Temperature: XX.X °F (XX.X °C), Humidity: XX.X %
```

If you see errors, check wiring.

### Test MQTT Publishing
1. Go to http://www.hivemq.com/demos/websocket-client/
2. Click "Connect"
3. Subscribe to topic: `adc-lawrence/temperature`
4. You should see current temperature appear

### Test Temperature Changes
1. Breathe on the DHT11 sensor
2. Wait 30 seconds for next reading
3. Serial monitor should show: "Temperature changed! Publishing to MQTT"
4. New value appears on web dashboard

## Monitoring

### Serial Monitor
```bash
pio device monitor
```

Shows:
- Boot sequence
- WiFi connection status
- MQTT connection status
- Sensor readings every 30 seconds
- Publishing events

### MQTT Monitoring

Use any MQTT client to subscribe:
- **Host**: broker.hivemq.com
- **Port**: 1883
- **Topics**: 
  - `adc-lawrence/temperature` (temperature in °F)
  - `adc-lawrence/ip` (device local IP)

### Web Dashboard

Open the public dashboard to monitor:
- Current temperature
- Last update time
- Device IP address
- Connection status

## Common Issues

### "Failed to read from DHT sensor!"
- Check DHT11 wiring
- Ensure stable 3.3V power supply
- DHT11 may need 2 seconds to initialize after power-on

### "WiFi connection failed"
- Verify SSID and password in `credentials.h`
- Check 2.4GHz WiFi is available (ESP8266 doesn't support 5GHz)
- Move device closer to router

### "MQTT connection failed"
- Check internet connectivity
- Verify device has WiFi connection
- HiveMQ public broker may occasionally be down (rare)

### Temperature not updating on web dashboard
- Check device is publishing (serial monitor)
- Verify MQTT connection status
- Try refreshing web page
- Check browser console for errors

## Production Deployment

### Power Supply
- USB power adapter (5V, 500mA minimum)
- Or USB power bank for portable operation

### Enclosure
Consider housing in a case with:
- Ventilation holes (don't block DHT11 sensor)
- Cable management for USB power
- Access to USB port for firmware updates

### Placement
- Away from direct heat sources
- Good air circulation around sensor
- Within WiFi range
- Near power outlet

### Monitoring
- Check device IP daily for first week
- Verify MQTT publishing frequency
- Monitor temperature readings for anomalies

## Maintenance

### Regular Checks
- Weekly: Verify device is online and publishing
- Monthly: Check WiFi signal strength
- As needed: Dust sensor area gently

### Firmware Updates
```bash
# Pull latest code
git pull

# Build and upload
pio run --target upload
```

### WiFi Reconfiguration
To change WiFi settings:
1. Update `credentials.h`
2. Re-upload firmware

## Support

For issues or questions:
1. Check serial monitor output
2. Review common issues above
3. Verify wiring and power supply
4. Test with known-good WiFi network

## Next Steps

- [ ] Consider adding temperature threshold alerts
- [ ] Implement data logging to cloud service
- [ ] Add more sensors (humidity trends, air quality)
- [ ] Create mobile app for monitoring
- [ ] Setup email/SMS notifications for temperature anomalies
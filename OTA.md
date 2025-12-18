# OTA Firmware Updates Guide

Over-the-Air (OTA) updates allow you to update the device firmware wirelessly without needing to connect via USB. Perfect for remote devices!

## Prerequisites

1. Device must be connected to WiFi
2. Device IP address (check serial monitor or `adc-lawrence/ip` MQTT topic)
3. PlatformIO installed with the project

## Method 1: OTA via PlatformIO CLI (Recommended)

### Step 1: Update platformio.ini

Edit `platformio.ini` and add your device IP in the `[env:d1_mini]` section:

```ini
[env:d1_mini]
platform = espressif8266
board = d1_mini
framework = arduino
monitor_speed = 115200
upload_protocol = espota
upload_port = 192.168.1.202    ; Update with your device's IP address
lib_deps = ...
```

### Step 2: Build and Upload

```bash
# Build the firmware
pio run

# Upload via OTA (no USB cable needed!)
pio run --target upload

# Monitor serial output after upload
pio device monitor
```

**That's it!** The device will:
1. Download the new firmware over WiFi
2. Validate and install it
3. Automatically reboot with the new firmware

### Step 3: Verify Update

Check the serial monitor output:
```
OTA Update starting...
OTA Progress: 10%
OTA Progress: 20%
...
OTA Progress: 100%
OTA Update complete!
Device rebooting...

=================================
Lawrence Office Thermometer
=================================

DHT11 sensor initialized on pin D7
Starting WiFi configuration...
Connecting to WiFi: sliderule
........

WiFi connected successfully!
```

## Method 2: Using Arduino IDE (Alternative)

If using Arduino IDE instead of VS Code:

1. Go to **Tools** → **Programmer** → Select **esptool**
2. Set **Tools** → **Network Port** to `192.168.1.202`
3. Upload as normal (Ctrl+U)

## Troubleshooting OTA Updates

### "Failed to connect to device"
- Verify device IP address is correct
- Ensure device is on the same WiFi network as your computer
- Check device WiFi connection (should show in serial monitor)
- Try pinging the device: `ping 192.168.1.202`

### "OTA receive error"
- Device may have run out of memory during upload
- Try uploading a smaller firmware first
- Restart device and try again

### "Upload timed out"
- Device may be too far from WiFi router
- Try moving closer to router
- Check WiFi signal strength in device's local web page

### Need to revert to USB upload?
Remove the OTA configuration from `platformio.ini`:
```ini
upload_protocol = esptool  ; Change back to serial/USB
# Remove or comment out: upload_port = ...
```

Then upload via USB cable as usual.

## Important Security Notes

OTA updates are sent **unencrypted** over your local network. This is fine for home/office use, but:

1. **Never expose OTA to the internet** - Only use on private networks
2. **Keep your WiFi password strong** - OTA access requires WiFi access
3. **Update device credentials** if WiFi password changes - Re-upload firmware with new `credentials.h`

## Version Management

When making OTA updates, consider:

1. **Test locally first** - Always build and test with USB connection before OTA
2. **Keep backups** - Save previous firmware binaries in case you need to roll back
3. **Update documentation** - If adding/removing MQTT topics, update this README

## Automatic Update Checking (Future Feature)

You could implement:
- Version checking from a remote server
- Automatic OTA update notifications
- Firmware checksum validation
- Rollback on failed update

See `src/main.cpp` for extension points.

## Storage Considerations

The ESP8266 has:
- **1MB Flash** - Split between bootloader, firmware, and SPIFFS
- **Current firmware** uses ~340KB of Flash (~32.5%)
- **Available for growth** - About 600KB remaining before size constraints

Large firmware updates (>600KB) may require:
- Removing unused libraries
- Compressing code
- Disabling features

## Restarting Device via OTA

After a successful OTA update, the device automatically restarts. To manually restart:

1. Power cycle the device (disconnect/reconnect power)
2. Or wait for watchdog timer to trigger (if needed)

## Tips & Tricks

### Find device IP quickly:
```bash
# Option 1: Serial monitor
pio device monitor

# Option 2: Check MQTT
mosquitto_sub -h broker.hivemq.com -t "adc-lawrence/ip"

# Option 3: Router admin panel
# Look for "lawrence-thermometer" or device name

# Option 4: Network scanner
nmap -sn 192.168.1.0/24 | grep -i esp
```

### Automate updates:
Create a script `update.sh`:
```bash
#!/bin/bash
DEVICE_IP="192.168.1.202"
sed -i "" "s/upload_port = .*/upload_port = $DEVICE_IP/" platformio.ini
pio run --target upload
```

Then just run: `./update.sh`

### Watch device during update:
In one terminal, start monitoring:
```bash
pio device monitor
```

In another terminal, upload:
```bash
pio run --target upload
```

This shows the real-time update progress.

## Support

For issues:
1. Check troubleshooting section above
2. Verify device is responding (check local web page)
3. Try USB upload to verify device still works
4. Check device serial monitor for error messages

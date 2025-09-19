# WakeDHT - Temperature, Humidity and WoL

![alt text](/photos/dashboard2.png)

## Description

An app based on [SimpleDHT library](https://github.com/winlinvip/SimpleDHT) for the DHT series of low-cost temperature & humidity sensors, now featuring **real-time updates**, **computer monitoring**, and **Wake-on-LAN** capabilities with optimized performance.

You can find DHT11 and DHT22 tutorials [here](https://learn.adafruit.com/dht).

## 🛠️ Quick Setup Guide

### Prerequisites
- **[Arduino CLI](https://docs.arduino.cc/arduino-cli/installation/)** for building and uploading the code to the board

- mkspiffs + esptool, usually installed with Arduino CLI and located at:
  - C:\Users\YourName\AppData\Local\Arduino15\packages\esp32\tools\esptool_py\5.0.0\esptool.exe
  - C:\Users\YourName\AppData\Local\Arduino15\packages\esp32\tools\mkspiffs\0.2.3\mkspiffs.exe

### 1. Hardware Setup
- **DHT11/DHT22 Sensor:** Connect to pin 14 on ESP32
- **ESP32:** Any ESP32 board with Wi-Fi and SPIFFS (tested on ESP32-S2 mini)
- **Breadboard and jumper wires:** For circuit connections (or just solder everything)

Below, you can find the **schematic** and an example using DHT11 and ESP32-S2 mini:

<img alt="schematic" src="/photos/schematic.png" width="600px" /><img align="right" alt="example" src="/photos/example.jpg" width="200px"/>

### 2. Software Setup

#### Install ESP32 board & libraries (arduino-cli)

1. Install [Arduino CLI](https://arduino.github.io/arduino-cli/latest/installation/)

2. Initialize config and add Espressif package index:
```powershell
arduino-cli config init
arduino-cli config set board_manager.additional_urls "https://espressif.github.io/arduino-esp32/package_esp32_index.json"
```

3. Update indexes and install ESP32 core:
```powershell
arduino-cli core update-index
arduino-cli core install esp32:esp32
arduino-cli core list
```

4. Install required libraries (includes dependencies):
```powershell
arduino-cli lib update-index
arduino-cli lib install "SimpleDHT"
arduino-cli lib list
```

### 3. Configuration

#### WiFi Credentials
1. Update the [secrets.h](secrets.h) file with your Wi-Fi details:
  ```cpp
  const char *ssid = "<YOUR_SSID>";
  const char *password = "<YOUR_PASSWORD>";
  ```

#### Computer Configuration (Optional)
Edit the computers array in [data/script.js](data/script.js):
  ```javascript
  const computers = [
    { name: 'device1', mac: 'AA-BB-CC-DD-EE-00', ip: '192.168.1.100', port: 22 },
    { name: 'device2', mac: 'AA-BB-CC-DD-EE-01', ip: '192.168.1.101', port: 22 }
  ];
  ```

### 4. Build and Upload
1. Verify board FQBN and connected port:
```powershell
arduino-cli board listall | grep esp32   # find available FQBNs for your board
arduino-cli board list                           # shows connected ports (e.g. /dev/ttyUSB0 or COM9)
```

2. Compile and upload firmware (replace <FQBN> and <PORT>):
```powershell
# Compile
arduino-cli compile --fqbn <FQBN> ./WakeDHT

# Upload
arduino-cli upload -p <PORT> --fqbn <FQBN> ./WakeDHT
```

3. SPIFFS / web files upload (mkspiffs + esptool)
- Generate spiffs image from data/:
```powershell
# Navigate to project directory
cd "C:\Users\YourName\Projects\WakeDHT"

# Generate SPIFFS image
C:\Users\YourName\AppData\Local\Arduino15\packages\esp32\tools\mkspiffs\0.2.3\mkspiffs.exe -c data -b 4096 -p 256 -s 0x160000 spiffs.bin

# Upload SPIFFS to board (note: use default-reset and hard-reset with hyphens)
C:\Users\YourName\AppData\Local\Arduino15\packages\esp32\tools\esptool_py\5.0.0\esptool.exe --chip esp32s2 --port COM9 --baud 921600 --before default-reset --after hard-reset write-flash -z 0x290000 spiffs.bin
```
(You can use the mkspiffs and esptool executables included with your platform package or download them separately.)

Notes:
- Replace paths, <FQBN> and <PORT> with values from your system. For Windows use COM ports (e.g. COM9).

- You can check my commands explained [here](UPLOAD.md)

### 5. Access Your Device

1. **Serial Monitor:** Check connection status and IP address
2. **Web Interface:**
  - mDNS: `http://esp32.local` (may not work on all networks but easier)
  - Local IP: `http://192.168.1.XXX` (check serial monitor or your router's admin dashboard)

## 📡 API Endpoints

The ESP32 provides RESTful API endpoints:

- **GET `/api`** - Temperature and humidity data
  ```json
  {"temperature":25,"humidity":60,"valid":true}
  ```

- **GET `/ping?ip=192.168.1.100`** - Check computer status
  ```json
  {"online":true}
  ```

- **GET `/wol?mac=AA-BB-CC-DD-EE-FF`** - Wake computer
  ```json
  {"success":true}
  ```

## 🔧 Troubleshooting

### Common Issues

**1. Upload Failed / Port Not Found**
- Check USB cable and drivers
- Press BOOT button during upload (if required)
- Try different baud rate: 115200

**2. WiFi Connection Issues**
- Verify `secrets.h` credentials
- Check 2.4GHz network (ESP32 doesn't support 5GHz)
- Monitor serial output for connection status

**3. Sensor Reading Issues**
- Check DHT11/DHT22 wiring
- Verify pin assignment (default: pin 14)
- Replace sensor if consistently invalid

**4. Web Interface Not Loading**
- Check SPIFFS upload was successful
- Verify file paths in `data/` folder
- Check browser console for errors

**5. Computer Detection/WOL Issues**
- Ensure computers are on same network
- Verify MAC addresses and IP addresses
- Check firewall settings on target computers
- Enable Wake-on-LAN in BIOS/UEFI

## 📁 Project Structure

```
WakeDHT/
├── WakeDHT.ino       # Main firmware code
├── secrets.h                # WiFi credentials (create this)
├── data/                      # Web interface files
│   ├── index.html       # Main UI
│   ├── script.js            # JavaScript functionality
│   ├── favicon.svg      # Icon
│   └── manifest.json   # PWA manifest
├── photos/                   # Documentation images
├── LICENSE              # License file
└── README.m          # This file
```

## Credits

The author and maintainer of this app is DynoW <contact@stefan.is-a.dev>.

Based on previous work of Winlin <winlin@vip.126.com>, the author of SimpleDHT library.

## Links

1. [adafruit/DHT-sensor-library](https://github.com/adafruit/DHT-sensor-library)
2. [Arduino #4469: Add SimpleDHT library.](https://github.com/arduino/Arduino/issues/4469)
3. [DHT11 datasheet and protocol.](https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf)
4. [DHT22 datasheet and protocol.](https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT22.pdf)

## License

This app is licensed under [MIT](https://github.com/DynoW/WakeDHT/blob/master/LICENSE) license.
# SimpleDHT-app - Temperature and humidity on the web!

## Description

An app based on [SimpleDHT library](https://github.com/winlinvip/SimpleDHT) for the DHT series of low-cost temperature & humidity sensors.

You can find DHT11 and DHT22 tutorials [here](https://learn.adafruit.com/dht).

## Hardware Components

- **DHT11 or DHT22 Sensor:** Measures temperature and humidity.
- **ESP32:** Microcontroller to read sensor data and host a webserver.
- **Breadboard and jumper wires:** For circuit connections. (Or just solder everything)

## Features

- **Temperature and Humidity Monitoring:** Real-time temperature and humidity data from DHT11 to any device!
- **Documentation:** Tons of descriptive comments and helpful information.
- **Customizable:** You can change everything from how the app works and integrates with the sensor to how it looks on your devices.
- **Raw files:** Access to raw HTML, JavaScript and Tailwind CSS files [here](https://github.com/DynoW/SimpleDHT-app/blob/main/raw).
- **MIT license:** This app is open-source and uses one of the most permissive licenses so you can use it on any project.

## Connections

Below, you can find the **schematic** and an example using DHT11 and ESP32-S2 mini:

<img alt="schematic" src="/photos/schematic.png" width="600px" /><img align="right" alt="example" src="/photos/example.jpg" width="200px"/>

## Library Installation

### First Method

![image](https://user-images.githubusercontent.com/36513474/68069796-09e62200-fd87-11e9-81e0-dc75e38efed0.png)

1. In the Arduino IDE, navigate to Sketch > Include Library > Manage Libraries
1. Then the Library Manager will open and you will find a list of libraries that are already installed or ready for installation.
1. Then search for SimpleDHT using the search bar.
1. Click on the text area and then select the specific version and install it.

### Second Method

1. Navigate to the [Releases page](https://github.com/winlinvip/SimpleDHT/releases).
1. Download the latest release.
1. Extract the zip file
1. In the Arduino IDE, navigate to Sketch > Include Library > Add .ZIP Library

## Usage

### Sep 1

Copy the [SimpleDHT-app.ino](https://github.com/DynoW/SimpleDHT-app/blob/main/SimpleDHT-app.ino) file into your IDE.

### Sep 2

Create a [secrets.h](https://github.com/DynoW/SimpleDHT-app/blob/main/secrets.h) file with the template below

```cpp
const char *ssid = "<AP_SSID>";
const char *password = "<AP_PASSWORD>";
```

> Remark: Replace <AP_SSID> and <AP_PASSWORD> with your access point credentials.

### Sep 3

and a [ci.json](https://github.com/DynoW/SimpleDHT-app/blob/main/ci.json) file as follows:

```json
{
  "requires": [
    "CONFIG_SOC_WIFI_SUPPORTED=y"
  ]
}
```

### Final step

Upload to your microcontroller! ▶️

## Links

1. [adafruit/DHT-sensor-library](https://github.com/adafruit/DHT-sensor-library)
2. [Arduino #4469: Add SimpleDHT library.](https://github.com/arduino/Arduino/issues/4469)
3. [DHT11 datasheet and protocol.](https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf)
4. [DHT22 datasheet and protocol.](https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT22.pdf)

## Credits

The author and maintainer of this app is DynoW <contact@stefan.is-a.dev>.

Based on previous work of Winlin <winlin@vip.126.com>, the author of SimpleDHT library:

## License

This app is licensed under [MIT](https://github.com/DynoW/SimpleDHT-app/blob/master/LICENSE) license.

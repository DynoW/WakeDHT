# This tutorial is intended to give you my way of uploading the code to the board

- Show the installed arduino-cli version.
```powershell
    arduino-cli version
``` 

- Initialize a default arduino-cli config
```bash
arduino-cli config init
```

- Add the ESP32 board manager URL
```bash
arduino-cli config add board_manager.additional_urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

- Update the local cores index
```bash
arduino-cli core update-index
```

- Install the ESP32 Arduino core
```bash
arduino-cli core install esp32:esp32
```

- Install the SimpleDHT library required by the sketch
```bash
arduino-cli lib install "SimpleDHT"
```

- List connected boards and available serial ports
```bash
arduino-cli board list
```

- Compile WakeDHT.ino for LoLin S2 Mini with specific partition and upload size
```bash
arduino-cli compile --fqbn esp32:esp32:lolin_s2_mini --build-property "build.partitions=default" --build-property "upload.maximum_size=1048576" WakeDHT.ino
```

- Upload the compiled sketch (replace <PORT> with your serial port, e.g., COM3 or /dev/ttyUSB0)
```bash
arduino-cli upload --fqbn esp32:esp32:lolin_s2_mini --port <PORT> WakeDHT.ino
```

- Create a SPIFFS image from the data folder
```bash
mkspiffs -c data -b 4096 -p 256 -s 0x160000 spiffs.bin
```

- Flash the SPIFFS image to the device (replace <PORT> with your device port)
```bash
esptool.py --chip esp32s2 --port <PORT> --baud 921600 --before default_reset --after hard_reset write_flash -z 0x290000 spiffs.bin
```
# This tutorial is intended to give you my way of uploading the code to the board

- `arduino-cli version`  
    Show the installed arduino-cli version.

- `arduino-cli config init`  
    Create a default arduino-cli config file.

- `arduino-cli config add board_manager.additional_urls https://espressif.github.io/arduino-esp32/package_esp32_index.json`  
    Add the ESP32 board manager URL so esp32 cores are available.

- `arduino-cli core update-index`  
    Update the local cores index from configured board manager URLs.

- `arduino-cli core install esp32:esp32`  
    Install the ESP32 Arduino core.

- `arduino-cli lib install "SimpleDHT"`  
    Install the SimpleDHT library required by the sketch.

- `arduino-cli board list`  
    List connected boards and available serial ports.

- `arduino-cli compile --fqbn esp32:esp32:lolin_s2_mini --build-property "build.partitions=default" --build-property "upload.maximum_size=1048576" WakeDHT.ino`  
    Compile WakeDHT.ino for the LoLin S2 Mini board, setting partition table and maximum upload size.

- `arduino-cli upload --fqbn esp32:esp32:lolin_s2_mini --port <PORT> WakeDHT.ino`  
    Upload the compiled sketch to the board. Replace `<PORT>` with your serial port (e.g., `COM3` or `/dev/ttyUSB0`).

- `mkspiffs -c data -b 4096 -p 256 -s 0x160000 spiffs.bin`  
    Create a SPIFFS image (`spiffs.bin`) from the `data` folder. Tool executable path varies by platform; this shows the generic invocation.

- `esptool.py --chip esp32s2 --port <PORT> --baud 921600 --before default_reset --after hard_reset write_flash -z 0x290000 spiffs.bin`  
    Flash the SPIFFS image to the device at address `0x290000`. Replace `<PORT>` with your device port.
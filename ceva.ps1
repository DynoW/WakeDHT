arduino-cli version
arduino-cli board list
arduino-cli compile --fqbn esp32:esp32:lolin_s2_mini --build-property "build.partitions=default" --build-property "upload.maximum_size=1048576" SimpleDHT-app.ino
arduino-cli upload --fqbn esp32:esp32:lolin_s2_mini --port COM7 SimpleDHT-app.ino
C:\Users\naffe\AppData\Local\Arduino15\packages\esp32\tools\mkspiffs\0.2.3\mkspiffs.exe -c data -b 4096 -p 256 -s 0x160000 spiffs.bin
C:\Users\naffe\AppData\Local\Arduino15\packages\esp32\tools\esptool_py\4.9.dev3\esptool.exe --chip esp32s2 --port COM7 --baud 921600 write_flash -z 0x290000 spiffs.bin
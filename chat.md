arduino-cli version

pio --version

winget install ArduinoSA.CLI

refreshenv; arduino-cli version

"C:\Program Files\Arduino CLI\arduino-cli.exe" 

& "C:\Program Files\Arduino CLI\arduino-cli.exe" version

& "C:\Program Files\Arduino CLI\arduino-cli.exe" config init

& "C:\Program Files\Arduino CLI\arduino-cli.exe" config add board_manager.additional_urls https://espressif.github.io/arduino-esp32/package_esp32_index.json

& "C:\Program Files\Arduino CLI\arduino-cli.exe" core update-index

& "C:\Program Files\Arduino CLI\arduino-cli.exe" core install esp32:esp32

& "C:\Program Files\Arduino CLI\arduino-cli.exe" lib install "SimpleDHT"

& "C:\Program Files\Arduino CLI\arduino-cli.exe" board listall | Select-String "s2"

& "C:\Program Files\Arduino CLI\arduino-cli.exe" board list

cd "c:\Users\naffe\repos\SimpleDHT-app"; & "C:\Program Files\Arduino CLI\arduino-cli.exe" compile --fqbn esp32:esp32:lolin_s2_mini SimpleDHT-app.ino

arduino-cli upload --fqbn esp32:esp32:lolin_s2_mini --port COM8 SimpleDHT-app.ino
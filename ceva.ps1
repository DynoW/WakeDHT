# ESP32 WakeDHT Build and Upload Script
# This script automates the complete build and upload process using Arduino CLI
# 
# Prerequisites:
# - Arduino CLI installed and in PATH
# - ESP32 board support installed
# - SimpleDHT library installed
# - USB drivers for ESP32 board
#
# Usage: .\ceva.ps1
# Make sure to connect your ESP32 board to COM9 before running

Write-Host "=== ESP32 WakeDHT Build Script ===" -ForegroundColor Green
Write-Host "Starting automated build and upload process..." -ForegroundColor Yellow

# Step 1: Check Arduino CLI installation and version
Write-Host "`n[1/10] Checking Arduino CLI installation..." -ForegroundColor Cyan
try {
    arduino-cli version
    if ($LASTEXITCODE -ne 0) { throw "Arduino CLI not found" }
} catch {
    Write-Host "ERROR: Arduino CLI not installed or not in PATH" -ForegroundColor Red
    Write-Host "Please install Arduino CLI from: https://arduino.github.io/arduino-cli/" -ForegroundColor Yellow
    exit 1
}

# Step 2: Initialize Arduino CLI configuration
Write-Host "`n[2/10] Initializing Arduino CLI configuration..." -ForegroundColor Cyan
arduino-cli config init

# Step 3: Add ESP32 board manager URL
Write-Host "`n[3/10] Adding ESP32 board manager URL..." -ForegroundColor Cyan
arduino-cli config add board_manager.additional_urls https://espressif.github.io/arduino-esp32/package_esp32_index.json

# Step 4: Update board manager index
Write-Host "`n[4/10] Updating board manager index..." -ForegroundColor Cyan
arduino-cli core update-index

# Step 5: Install ESP32 board support
Write-Host "`n[5/10] Installing ESP32 board support..." -ForegroundColor Cyan
arduino-cli core install esp32:esp32

# Step 6: Install required libraries
Write-Host "`n[6/10] Installing SimpleDHT library..." -ForegroundColor Cyan
arduino-cli lib install "SimpleDHT"

# Step 7: List available boards for verification
Write-Host "`n[7/10] Detecting connected boards..." -ForegroundColor Cyan
arduino-cli board list

# Step 8: Compile the firmware
Write-Host "`n[8/10] Compiling firmware for ESP32-S2..." -ForegroundColor Cyan
Write-Host "Board: esp32:esp32:lolin_s2_mini" -ForegroundColor White
Write-Host "Partition: Default with SPIFFS support" -ForegroundColor White
try {
    arduino-cli compile --fqbn esp32:esp32:lolin_s2_mini --build-property "build.partitions=default" --build-property "upload.maximum_size=1048576" WakeDHT.ino
    if ($LASTEXITCODE -ne 0) { throw "Compilation failed" }
    Write-Host "✓ Firmware compiled successfully!" -ForegroundColor Green
} catch {
    Write-Host "ERROR: Firmware compilation failed!" -ForegroundColor Red
    Write-Host "Check for syntax errors in WakeDHT.ino" -ForegroundColor Yellow
    exit 1
}

# Step 9: Upload firmware to ESP32
Write-Host "`n[9/10] Uploading firmware to COM9..." -ForegroundColor Cyan
Write-Host "Make sure ESP32 is connected to COM9 and press BOOT button if needed" -ForegroundColor Yellow
try {
    arduino-cli upload --fqbn esp32:esp32:lolin_s2_mini --port COM9 WakeDHT.ino
    if ($LASTEXITCODE -ne 0) { throw "Upload failed" }
    Write-Host "✓ Firmware uploaded successfully!" -ForegroundColor Green
} catch {
    Write-Host "ERROR: Firmware upload failed!" -ForegroundColor Red
    Write-Host "Check USB connection, COM port, and try pressing BOOT button during upload" -ForegroundColor Yellow
    Write-Host "Available ports:" -ForegroundColor Yellow
    arduino-cli board list
    exit 1
}

# Step 10: Generate and upload SPIFFS (web interface files)
Write-Host "`n[10/10] Generating and uploading SPIFFS..." -ForegroundColor Cyan

# Generate SPIFFS image
Write-Host "Generating SPIFFS image from data/ folder..." -ForegroundColor White
try {
    & "C:\Users\$env:USERNAME\AppData\Local\Arduino15\packages\esp32\tools\mkspiffs\0.2.3\mkspiffs.exe" -c data -b 4096 -p 256 -s 0x160000 spiffs.bin
    if ($LASTEXITCODE -ne 0) { throw "SPIFFS generation failed" }
    Write-Host "✓ SPIFFS image generated successfully!" -ForegroundColor Green
} catch {
    Write-Host "ERROR: SPIFFS generation failed!" -ForegroundColor Red
    Write-Host "Check that data/ folder exists and contains web files" -ForegroundColor Yellow
    exit 1
}

# Upload SPIFFS to ESP32
Write-Host "Uploading SPIFFS to ESP32..." -ForegroundColor White
try {
    & "C:\Users\$env:USERNAME\AppData\Local\Arduino15\packages\esp32\tools\esptool_py\5.0.0\esptool.exe" --chip esp32s2 --port COM9 --baud 921600 --before default-reset --after hard-reset write-flash -z 0x290000 spiffs.bin
    if ($LASTEXITCODE -ne 0) { throw "SPIFFS upload failed" }
    Write-Host "✓ SPIFFS uploaded successfully!" -ForegroundColor Green
} catch {
    Write-Host "ERROR: SPIFFS upload failed!" -ForegroundColor Red
    Write-Host "Check USB connection and ensure ESP32 is still connected to COM9" -ForegroundColor Yellow
    exit 1
}

# Success message
Write-Host "`n=== BUILD AND UPLOAD COMPLETE ===" -ForegroundColor Green
Write-Host "Your ESP32 is now running the WakeDHT!" -ForegroundColor Green
Write-Host "`nNext steps:" -ForegroundColor Yellow
Write-Host "1. Open Serial Monitor (115200 baud) to see ESP32 IP address" -ForegroundColor White
Write-Host "2. Connect to WiFi network configured in secrets.h" -ForegroundColor White
Write-Host "3. Access web interface at http://esp32.local or IP address" -ForegroundColor White
Write-Host "4. Check temperature/humidity readings and computer management" -ForegroundColor White

Write-Host "`nTroubleshooting:" -ForegroundColor Yellow
Write-Host "- If upload fails: Press and hold BOOT button during upload" -ForegroundColor White
Write-Host "- If WiFi fails: Check credentials in secrets.h file" -ForegroundColor White
Write-Host "- If web interface doesn't load: Verify SPIFFS upload was successful" -ForegroundColor White
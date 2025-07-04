// Libraries for Wi-Fi & webserver
#include <WiFi.h>
#include <WebServer.h>

// Setup mDNS: http://esp32.local
#include <ESPmDNS.h>

// DTH11 & DHT22 open-source library !installation required! (https://github.com/winlinvip/SimpleDHT)
#include <SimpleDHT.h>

// For Wake on LAN
#include <WiFiUdp.h>

// SPIFFS for file system
#include <SPIFFS.h>

// Access point credentials
#include "secrets.h"

// For DHT11 module:
// Define the digital pin used to connect the module
int pinDHT11 = 14;
SimpleDHT11 dht11(pinDHT11);

// Declare two byte variables for temperature and humidity
byte temperature = 0;
byte humidity = 0;

// Define the webserver on port 80
WebServer server(80);

// UDP instance for Wake on LAN
WiFiUDP udp;

const unsigned long PING_TIMEOUT = 200;   // Very short ping timeout for responsiveness
unsigned long lastApiUpdate = 0;          // Timer for non-blocking sensor reads
const unsigned long API_UPDATE_INTERVAL = 2000; // Update sensor every 2 seconds

// Function to get MIME type based on file extension
String getMimeType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".json")) return "application/json";
  else if (filename.endsWith(".svg")) return "image/svg+xml";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

// Serve data (temperature + humidity)
void api() {
  // This function is now non-blocking and returns the last known values
  String data = "{\"temperature\": " + String(temperature) + ", \"humidity\": " + String(humidity) + "}";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", data);
}

// Advanced ping using multiple lightweight methods
void pingComputer() {
  yield(); // Allow background tasks to run

  String ip = server.arg("ip");
  bool isOnline = false;
  
  if (ip.length() > 0) {
    IPAddress target;
    if (target.fromString(ip)) {
      // Method 1: Quick TCP SYN probe (most reliable for active hosts)
      isOnline = quickTCPProbe(target);
      
      // Method 2: If TCP fails, try UDP echo
      if (!isOnline) {
        isOnline = udpEcho(target);
      }
    }
  }
  
  String response = "{\"online\":" + String(isOnline ? "true" : "false") + "}";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", response);

  yield(); // Allow background tasks to run
}

// Quick TCP probe with minimal timeout
bool quickTCPProbe(IPAddress target) {
  WiFiClient client;
  client.setTimeout(PING_TIMEOUT); // Very short timeout
  
  // Check only SSH (22) and TFTP (69) ports
  int ports[] = {22, 69}; // SSH, TFTP
  
  for (int i = 0; i < 2; i++) { // Try both ports
    if (client.connect(target, ports[i])) {
      client.stop();
      return true;
    }
    yield(); // Allow other tasks
  }
  
  return false;
}

// UDP echo test (works for many devices)
bool udpEcho(IPAddress target) {
  WiFiUDP udp;
  
  // Try to send to echo port (7) - many devices respond
  if (udp.begin(0)) { // Use any available local port
    udp.beginPacket(target, 7);
    udp.write((const uint8_t*)"ping", 4); // Send the string as a byte array
    bool sent = udp.endPacket();
    udp.stop();
    
    if (sent) {
      // If we could send the packet, the device is likely reachable
      // In a more sophisticated implementation, we would wait for a response
      return true;
    }
  }
  
  return false;
}

// Send Wake on LAN magic packet
void wakeOnLAN() {
  yield(); // Allow other tasks to run

  // Get MAC address from query parameter (to match JavaScript expectations)
  String macStr = server.arg("mac");
  
  if (macStr.length() == 0) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, "application/json", "{\"success\":false,\"message\":\"MAC address parameter missing\"}");
    return;
  }
  
  // Convert MAC string to bytes (format: XX-XX-XX-XX-XX-XX)
  byte mac[6];
  if (!macAddressToBytes(macStr, mac)) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid MAC address format\"}");
    return;
  }
  
  // Send Wake on LAN magic packet
  bool success = sendWOL(mac);
  
  String response = "{\"success\":" + String(success ? "true" : "false") + "}";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", response);

  yield(); // Allow other tasks to run
}

// Convert MAC address string to bytes
bool macAddressToBytes(String macStr, byte* mac) {
  // Replace "-" with ":" and remove any spaces
  macStr.replace("-", ":");
  macStr.replace(" ", "");
  macStr.toUpperCase();
  
  // Check if the MAC address is valid
  if (macStr.length() != 17) {
    Serial.println("Invalid MAC address length: " + String(macStr.length()));
    return false;
  }
  
  // Convert MAC address string to bytes
  for (int i = 0; i < 6; i++) {
    int index = i * 3;
    if (index + 1 >= macStr.length()) {
      Serial.println("MAC address format error at index: " + String(index));
      return false;
    }
    String byteString = macStr.substring(index, index + 2);
    long value = strtol(byteString.c_str(), NULL, 16);
    if (value < 0 || value > 255) {
      Serial.println("Invalid MAC byte value: " + byteString);
      return false;
    }
    mac[i] = (byte)value;
  }
  
  return true;
}

// Send Wake on LAN magic packet
bool sendWOL(byte* mac) {
  // Create magic packet
  byte magicPacket[102];
  
  // First 6 bytes of 0xFF
  for (int i = 0; i < 6; i++) {
    magicPacket[i] = 0xFF;
  }
  
  // Repeat MAC address 16 times
  for (int i = 0; i < 16; i++) {
    int offset = i * 6 + 6;
    for (int j = 0; j < 6; j++) {
      magicPacket[offset + j] = mac[j];
    }
  }
  
  // Print debug information
  Serial.print("Sending WOL packet to MAC: ");
  for (int i = 0; i < 6; i++) {
    if (i > 0) Serial.print(":");
    if (mac[i] < 16) Serial.print("0");
    Serial.print(mac[i], HEX);
  }
  Serial.println();
  
  // Send packet to broadcast address on port 9 (standard WOL port)
  udp.beginPacket(IPAddress(255, 255, 255, 255), 9);
  int bytesWritten = udp.write(magicPacket, sizeof(magicPacket));
  bool success = udp.endPacket() == 1;
  
  Serial.println("WOL packet sent: " + String(success ? "Success" : "Failed"));
  Serial.println("Bytes written: " + String(bytesWritten));
  
  return success;
}

// Serve 404 page
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  // List arguments
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

// Tries to serve a file from SPIFFS, otherwise returns 404
void handleFileRequest() {
  String path = server.uri();
  if (path.endsWith("/")) {
    path += "index.html";
  }
  
  String mimeType = getMimeType(path);
  
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    server.streamFile(file, mimeType);
    file.close();
  } else {
    handleNotFound();
  }
}

void setup(void) {
  Serial.begin(115200);
  
  // Create routes
  server.on("/api", api);
  server.on("/ping", pingComputer);
  server.on("/wol", wakeOnLAN);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  Serial.println("SPIFFS mounted successfully");
  
  // List files in SPIFFS for debugging
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  Serial.println("Files in SPIFFS:");
  while (file) {
    Serial.print("  ");
    Serial.print(file.name());
    Serial.print(" (");
    Serial.print(file.size());
    Serial.println(" bytes)");
    file = root.openNextFile();
  }
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Set up http://esp32.local
  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.onNotFound(handleFileRequest);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();

  // Non-blocking sensor read
  unsigned long currentTime = millis();
  if (currentTime - lastApiUpdate >= API_UPDATE_INTERVAL) {
    lastApiUpdate = currentTime;

    byte newTemp = 0;
    byte newHumidity = 0;
    if (dht11.read(&newTemp, &newHumidity, NULL) == SimpleDHTErrSuccess) {
      temperature = newTemp;
      humidity = newHumidity;
    }
  }

  // Yield to allow other tasks to run
  yield();
}

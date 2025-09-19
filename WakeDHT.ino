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

// FreeRTOS for multitasking
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Access point credentials
#include "secrets.h"

// For DHT11 module:
// Define the digital pin used to connect the module
int pinDHT11 = 14;
SimpleDHT11 dht11(pinDHT11);

// Declare two byte variables for temperature and humidity (volatile for thread safety)
volatile byte temperature = 0;
volatile byte humidity = 0;
volatile bool sensorDataValid = false;

// Define the webserver on port 80
WebServer server(80);

// UDP instance for Wake on LAN
WiFiUDP udp;

const unsigned long PING_TIMEOUT = 100;   // Ultra-fast ping timeout
unsigned long lastClientHandle = 0;       // For rate limiting client handling
const unsigned long CLIENT_HANDLE_INTERVAL = 5; // Handle clients every 5ms for better responsiveness

// Task handles for FreeRTOS
TaskHandle_t sensorTaskHandle = NULL;

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

// Handle CORS preflight (OPTIONS) requests
void handleCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Cache-Control, Content-Type");
  server.send(204);
}

// Optimized API response with minimal string operations
void api() {
  // Use pre-allocated buffer for efficiency
  char jsonBuffer[80];
  snprintf(jsonBuffer, sizeof(jsonBuffer), 
           "{\"temperature\":%d,\"humidity\":%d,\"valid\":%s}", 
           temperature, humidity, sensorDataValid ? "true" : "false");
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Headers", "Cache-Control, Content-Type");
  server.send(200, "application/json", jsonBuffer);
}

// TCP probe with configurable port and improved timeout
bool quickTCPProbe(IPAddress target, int port) {
  WiFiClient client;
  client.setTimeout(PING_TIMEOUT);
  
  bool connected = client.connect(target, port);
  if (connected) {
    client.stop();
  }
  
  return connected;
}

// Optimized ping with configurable port - improved timeout for better offline detection
void pingComputer() {
  String ip = server.arg("ip");
  String portStr = server.arg("port");
  bool isOnline = false;
  
  if (ip.length() > 0) {
    IPAddress target;
    if (target.fromString(ip)) {
      // Default to port 22 (SSH) if no port specified
      int port = 22;
      if (portStr.length() > 0) {
        port = portStr.toInt();
        // Validate port range
        if (port < 1 || port > 65535) {
          port = 22; // Fall back to default
        }
      }
      
      // TCP probe to specified port
      isOnline = quickTCPProbe(target, port);
    }
  }
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Headers", "Cache-Control, Content-Type");
  server.send(200, "application/json", 
              "{\"online\":" + String(isOnline ? "true" : "false") + "}");
}

// Optimized MAC address parsing with minimal string operations
bool macAddressToBytes(const String& macStr, byte* mac) {
  // Quick validation - must be exactly 17 characters
  if (macStr.length() != 17) return false;
  
  // Parse each byte directly without string manipulation
  for (int i = 0; i < 6; i++) {
    int index = i * 3;
    char highNibble = macStr.charAt(index);
    char lowNibble = macStr.charAt(index + 1);
    
    // Convert hex characters to values
    byte high = (highNibble >= '0' && highNibble <= '9') ? highNibble - '0' :
                (highNibble >= 'A' && highNibble <= 'F') ? highNibble - 'A' + 10 :
                (highNibble >= 'a' && highNibble <= 'f') ? highNibble - 'a' + 10 : 255;
    
    byte low = (lowNibble >= '0' && lowNibble <= '9') ? lowNibble - '0' :
               (lowNibble >= 'A' && lowNibble <= 'F') ? lowNibble - 'A' + 10 :
               (lowNibble >= 'a' && lowNibble <= 'f') ? lowNibble - 'a' + 10 : 255;
    
    if (high > 15 || low > 15) return false;
    
    mac[i] = (high << 4) | low;
  }
  
  return true;
}

// Ultra-efficient WOL packet creation and sending
bool sendWOL(const byte* mac) {
  // Create magic packet on stack (more efficient than heap allocation)
  byte magicPacket[102];
  
  // Fill first 6 bytes with 0xFF
  memset(magicPacket, 0xFF, 6);
  
  // Efficiently copy MAC 16 times using pointer arithmetic
  byte* ptr = magicPacket + 6;
  for (int i = 0; i < 16; i++) {
    memcpy(ptr, mac, 6);
    ptr += 6;
  }
  
  // Send to broadcast on standard WOL port
  udp.beginPacket(IPAddress(255, 255, 255, 255), 9);
  udp.write(magicPacket, sizeof(magicPacket));
  return udp.endPacket() == 1;
}

// Optimized Wake on LAN with minimal allocations
void wakeOnLAN() {
  String macStr = server.arg("mac");
  
  if (macStr.length() == 0) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, "application/json", "{\"success\":false,\"message\":\"MAC missing\"}");
    return;
  }
  
  byte mac[6];
  if (!macAddressToBytes(macStr, mac)) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid MAC\"}");
    return;
  }
  
  bool success = sendWOL(mac);
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", 
              success ? "{\"success\":true}" : "{\"success\":false}");
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
  server.on("/api", HTTP_GET, api);
  server.on("/ping", HTTP_GET, pingComputer);
  server.on("/wol", HTTP_GET, wakeOnLAN);
  // CORS preflight (OPTIONS) handlers
  server.on("/api", HTTP_OPTIONS, handleCORS);
  server.on("/ping", HTTP_OPTIONS, handleCORS);
  server.on("/wol", HTTP_OPTIONS, handleCORS);

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
  
  // Create sensor reading task on core 0 (web server runs on core 1)
  xTaskCreatePinnedToCore(
    sensorTask,           // Task function
    "SensorTask",         // Task name
    2048,                 // Stack size
    NULL,                 // Parameters
    1,                    // Priority
    &sensorTaskHandle,    // Task handle
    0                     // Core 0
  );
  
  Serial.println("Sensor task started on core 0");
}

// Optimized sensor task with precise timing
void sensorTask(void *parameter) {
  const TickType_t xFrequency = pdMS_TO_TICKS(2000); // Read every 2 seconds
  TickType_t xLastWakeTime = xTaskGetTickCount();
  
  while (true) {
    byte newTemp = 0;
    byte newHumidity = 0;
    
    // Read sensor (blocking only this task, not web server)
    int result = dht11.read(&newTemp, &newHumidity, NULL);
    
    if (result == SimpleDHTErrSuccess) {
      // Atomic updates for thread safety
      temperature = newTemp;
      humidity = newHumidity;
      sensorDataValid = true;
    }
    // Keep last valid values on error, no console spam
    
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// Optimized main loop with precise timing control
void loop(void) {
  static unsigned long lastHandle = 0;
  unsigned long currentTime = millis();
  
  // Handle clients at optimal intervals
  if (currentTime - lastHandle >= CLIENT_HANDLE_INTERVAL) {
    lastHandle = currentTime;
    server.handleClient();
  }
  
  // Minimal task yield for FreeRTOS efficiency
  vTaskDelay(1);
}

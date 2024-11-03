// Libraries for Wi-Fi & webserver
#include <WiFi.h>
#include <NetworkClient.h>
#include <WebServer.h>

// Setup mDNS: http://esp32.local
#include <ESPmDNS.h>

// DTH11 & DHT22 open-source library !Don't forget to install! (https://github.com/DynoW/SimpleDHT-app/tree/master?tab=readme-ov-file#library-installation)
#include <SimpleDHT.h>

// Access point credentials
#include "secrets.h"


// Define the digital pin used to connect the module (you can change it to any other digital pin)
// For DHT11 module:
int pinDHT11 = D7;
SimpleDHT11 dht11(pinDHT11);

// For DHT22 module:
int pinDHT22 = D7;
SimpleDHT22 dht22(pinDHT22);

// Declare two byte variables for temperature and humidity
byte temperature = 0;
byte humidity = 0;

// Define the webserver on port 80
WebServer server(80);

// index.html
String page = R"(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Ambient temperature and humidity</title>
  <script src="https://cdn.tailwindcss.com"></script>
</head>
<body class="flex flex-col items-center gap-10 mt-10 bg-gray-100">
  <div class="flex flex-row container justify-around">
    <!-- It will show weird on screens smaller than 400px width (fix: you can change size of the svg) -->
    <div>
      <h1 id="t-heading" class="text-2xl font-semibold text-center">Temperature :</h1>
      <div class="relative flex items-center justify-center h-52 w-52">
        <svg class="transform -rotate-90 w-full h-full">
          <circle cx="100" cy="100" r="90" fill="none" stroke="#e5e7eb" stroke-width="12" />
          <circle id="t" cx="100" cy="100" r="90" fill="none" stroke="#4caf50" stroke-width="12" stroke-linecap="round"
            stroke-dasharray="565.48" stroke-dashoffset="565.48" style="transition: stroke-dashoffset 1s ease;" />
        </svg>
        <div class="absolute text-2xl font-bold text-gray-700 text-center" id="t-text">0&#8451;</div>
      </div>
    </div>
    <div>
      <h1 id="h-heading" class="text-2xl font-semibold text-center">Humidity :</h1>
      <div class="relative flex items-center justify-center h-52 w-52">
        <svg class="transform -rotate-90 w-full h-full">
          <circle cx="100" cy="100" r="90" fill="none" stroke="#e5e7eb" stroke-width="12" />
          <circle id="h" cx="100" cy="100" r="90" fill="none" stroke="#4caf50" stroke-width="12" stroke-linecap="round"
            stroke-dasharray="565.48" stroke-dashoffset="565.48" style="transition: stroke-dashoffset 1s ease;" />
        </svg>
        <div class="absolute text-2xl font-bold text-gray-700 text-center" id="h-text">0&#37;</div>
      </div>
    </div>
  </div>
  <script src="/script.js"></script>
</body>
</html>
)";

// Serve the index.html
void htmlIndex() {
  server.send(200, "text/html", page);
}

// Serve the script.js
void htmlScript() {
  String script = "function updateProgress(value, id) {";
  // Get the circle and text elements by their IDs
  script += "const circle = document.getElementById(id);";
  script += "const text = document.getElementById(id + '-text');";
  // Calculate the circumference of the circle
  script += "const radius = circle.r.baseVal.value;";
  script += "const circumference = 2 * Math.PI * radius;";
  script += "let offset;";
  // Determine the offset based on the value and id
  script += "if (value < 0) {";
  script += "offset = circumference;";
  script += "} else if ((id === 't' && value > 40) || (id === 'h' && value > 100)) {";
  script += "offset = 0;";
  script += "} else { offset = circumference - (value / (id === 't' ? 40 : 100)) * circumference; }";
  // Set the stroke dash offset to create the progress effect
  script += "circle.style.strokeDashoffset = offset;";
  // Define the conditions for temperature and humidity
  script += "const conditions = {";
  script += "t: [";
  script += "{ max: 18, color: 'stroke-blue-500', label: '(Cold)' },";
  script += "{ max: 24, color: 'stroke-green-500', label: '(Comfortable)' },";
  script += "{ max: 28, color: 'stroke-yellow-500', label: '(Warm)' },";
  script += "{ max: 32, color: 'stroke-orange-500', label: '(Hot)' },";
  script += "{ max: Infinity, color: 'stroke-red-500', label: '(Very Hot)' }";
  script += "],";
  script += "h: [";
  script += "{ max: 30, color: 'stroke-blue-500', label: '(Dry)' },";
  script += "{ max: 60, color: 'stroke-green-500', label: '(Comfortable)' },";
  script += "{ max: 70, color: 'stroke-yellow-500', label: '(Humid)' },";
  script += "{ max: 80, color: 'stroke-orange-500', label: '(Very Humid)' },";
  script += "{ max: Infinity, color: 'stroke-red-500', label: '(Excessive)' }";
  script += "] };";
  // Find the appropriate condition based on the value
  script += "const condition = conditions[id].find(cond => (value <= cond.max));";
  // Remove previous color classes that start with 'stroke-'
  script += "circle.classList.forEach(cls => {";
  script += "if (cls.startsWith('stroke-')) {";
  script += "circle.classList.remove(cls);";
  script += "} });";
  // Add the new color class
  script += "circle.classList.add(condition.color);";
  // Update the text content with the value and condition label
  script += "text.innerHTML = value + (id === 't' ? '&#8451;' : '&#37') + '<br>' + condition.label; }";
  // Fetch data from the API and update the progress every 3 seconds
  script += "setInterval(async () => {";
  script += "try {";
  script += "const response = await fetch('/api');";
  script += "const data = await response.json();";
  script += "updateProgress(data.temperature, 't');";
  script += "updateProgress(data.humidity, 'h');";
  script += "} catch (error) {";
  script += "console.error('Error fetching data:', error);}";
  script += "}, 3000);";
  
  server.send(200, "application/javascript", script);
}


// Serve data (temperature + humidity)
void api() {
  String data = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
  server.send(200, "application/json", data);
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

void setup(void) {
  Serial.begin(115200);
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

  // Create routes
  server.on("/", htmlIndex);
  server.on("/script.js", htmlScript);
  server.on("/api", api);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(2);
  // Read data from the module
  dht11.read(&temperature, &humidity, NULL);
}

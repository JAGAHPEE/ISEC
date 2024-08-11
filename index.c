#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>

#define SERVO_PIN_1 12  // ESP32 pin GPIO12 connected to first servo motor
#define SERVO_PIN_2 13  // ESP32 pin GPIO13 connected to second servo motor
#define RELAY_PIN 2      // Pin for controlling the relay

Servo servo1;
Servo servo2;

const char* ssid = "Power_Grid";      // Change it to your WiFi network's SSID
const char* password = "Volts&Amps";  // Change it to your WiFi network's password

WebServer server(80);  // Initialize WebServer on port 80

const char* HTML_CONTENT = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
body { text-align: center; }
.button { display: inline-block; margin: 10px; }
.button button { padding: 20px 40px; font-size: 24px; cursor: pointer; } /* Adjust padding and font-size here */
</style>
<script>
var ws;

function init() {
  ws = new WebSocket("ws://" + window.location.host + ":81");
  document.getElementById("ws_state").innerHTML = "CONNECTING";

  ws.onopen  = function(){ document.getElementById("ws_state").innerHTML = "CONNECTED" };
  ws.onclose = function(){ document.getElementById("ws_state").innerHTML = "CLOSED"};
  ws.onerror = function(){ alert("websocket error " + this.url) };
}

function setAngle(angle) {
  ws.send(angle.toString());
}

function showFlag() {
  document.getElementById("flag").innerHTML = "flag3{P0w3r_2_ISEA}";
}

window.onload = init;
</script>
</head>

<body>

<h2>Power Grid<br>

<div class="button">
  <button onclick="setAngle(0); showFlag();">ON</button>
</div>

<div class="button">
  <button onclick="setAngle(90); showFlag();">OFF</button>
</div>

<div id="flag"></div>

</h2>
<img align="bottom" src="https://media.assettype.com/newindianexpress%2F2024-02%2F2370a1ae-75a1-43e3-b38a-bc920054bf53%2FC_DAC.jpg?w=480&auto=format%2Ccompress&fit=max" alt=" ">
</body>
</html>
)=====";

void handleSetAngle() {
  if (server.hasArg("value")) {
    int angle = server.arg("value").toInt();
    Serial.print("Received angle: ");
    Serial.println(angle);

    // Adjust servo positions and relay control based on angle received
    if (angle == 0) {
      servo1.write(10);
      servo2.write(10);
      delay(500);
      digitalWrite(RELAY_PIN, LOW);  // Turn on relay
    } else if (angle == 90) {
      servo1.write(100);
      servo2.write(100);
      digitalWrite(RELAY_PIN, HIGH);  // Turn off relay
    } else {
      Serial.println("Invalid angle received.");
    }
  }
  server.send(200, "text/plain", "Angle set");
}

void setup() {
  Serial.begin(9600);
  servo1.attach(SERVO_PIN_1);  // Attach servo1 to GPIO 12
  servo2.attach(SERVO_PIN_2);  // Attach servo2 to GPIO 13
  pinMode(RELAY_PIN, OUTPUT);   // Initialize relay pin

  // Initialize servos to initial position
  servo1.write(10);
  servo2.write(10);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Handle HTTP requests
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", HTML_CONTENT);
  });
  
  server.on("/setAngle", HTTP_GET, handleSetAngle);

  server.begin();
  Serial.print("ESP32 Web Server's IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();  // Handle HTTP requests
}

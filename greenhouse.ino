#include <WiFi.h>
#include "DHT.h"

// ----------- WiFi -----------
const char* ssid = "Galaxy A06 4729";
const char* password = "t9dgdph55254yxv";

WiFiServer server(80);

// ----------- DHT -----------
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ----------- LEDs -----------
#define COOL_LED 23
#define HEAT_LED 22
#define MOIST_LED 21
#define LIGHT_LED 18

// ----------- Sensors -----------
#define MOISTURE 34
#define LDR_PIN 35

// ----------- States -----------
bool coolState = false;
bool heatState = false;
bool pumpState = false;
bool lightState = false;

// ----------- LDR Average -----------
int readLDR() {
  int total = 0;
  for (int i = 0; i < 10; i++) {
    total += analogRead(LDR_PIN);
    delay(10);
  }
  return total / 10;
}

void setup() {
  Serial.begin(115200);

  dht.begin();

  pinMode(COOL_LED, OUTPUT);
  pinMode(HEAT_LED, OUTPUT);
  pinMode(MOIST_LED, OUTPUT);
  pinMode(LIGHT_LED, OUTPUT);

  // WiFi connect
  WiFi.begin(ssid, password);
  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {

  float temp = dht.readTemperature();
  int moisture = analogRead(MOISTURE);
  int light = readLDR();

  // -------- Control Logic --------
  if (temp > 30) coolState = true;
  else if (temp <= 28) coolState = false;

  if (temp < 20) heatState = true;
  else if (temp >= 22) heatState = false;

  if (moisture < 1900) pumpState = true;
  else if (moisture >= 2000) pumpState = false;

  if (light > 800) lightState = false;
  else if (light > 500) lightState = true;
  else lightState = false;

  digitalWrite(COOL_LED, coolState);
  digitalWrite(HEAT_LED, heatState);
  digitalWrite(MOIST_LED, pumpState);
  digitalWrite(LIGHT_LED, lightState);

  // -------- Web Server --------
  WiFiClient client = server.available();

  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    // HTML page
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();

    client.println("<html><head><meta http-equiv='refresh' content='2'></head><body>");
    client.println("<h1>🌿 Smart Greenhouse</h1>");

    client.println("<p>Temperature: " + String(temp) + " °C</p>");
    client.println("<p>Moisture: " + String(moisture) + "</p>");
    client.println("<p>Light: " + String(light) + "</p>");

    client.println("<h3>Status:</h3>");
    client.println("<p>Cooling: " + String(coolState ? "ON" : "OFF") + "</p>");
    client.println("<p>Heating: " + String(heatState ? "ON" : "OFF") + "</p>");
    client.println("<p>Pump: " + String(pumpState ? "ON" : "OFF") + "</p>");
    client.println("<p>Light: " + String(lightState ? "ON" : "OFF") + "</p>");

    client.println("</body></html>");
    client.stop();
  }
}
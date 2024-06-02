#include <ESP8266WiFi.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// DHT Sensor settings
#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Relay settings
#define RELAY_PIN D5

// OLED screen settings
#define TFT_CS D1
#define TFT_RST D2
#define TFT_DC D3
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

const char* ssid = "ARRIS-44C9";
const char* password = "7A49DD21AA6C191B";

WiFiServer server(80);

unsigned long relayOffTime = 0;
bool relayState = false;
bool humidityControl = false;

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Ensure relay is off at startup

  // Initialize OLED screen
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  // Handle relay off timing
  if (relayState && millis() > relayOffTime) {
    digitalWrite(RELAY_PIN, LOW);
    relayState = false;
    humidityControl = false;
  }

  // Update sensor readings
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Display sensor readings on OLED
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.print("Humidity: ");
  tft.println(humidity);
  tft.print("%");
  tft.setCursor(0, 20);
  tft.print("Temperature: ");
  tft.println(temperature);
  tft.print("C");

  // Check if humidity control is active
  if (humidityControl && humidity >= 80.0) {
    digitalWrite(RELAY_PIN, LOW);
    relayState = false;
    humidityControl = false;
  }

  // Handle web client requests
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  while(!client.available()){
    delay(1);
  }

  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  if (req.indexOf("/relay/on/1min") != -1) {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = true;
    relayOffTime = millis() + 60000;
  } else if (req.indexOf("/relay/on/3min") != -1) {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = true;
    relayOffTime = millis() + 180000;
  } else if (req.indexOf("/relay/on/5min") != -1) {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = true;
    relayOffTime = millis() + 300000;
  } else if (req.indexOf("/relay/on/10min") != -1) {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = true;
    relayOffTime = millis() + 600000;
  } else if (req.indexOf("/relay/on/humidity") != -1) {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = true;
    humidityControl = true;
  } else if (req.indexOf("/relay/off") != -1) {
    digitalWrite(RELAY_PIN, LOW);
    relayState = false;
    humidityControl = false;
  } else if (req.indexOf("/data") != -1) {
    String data = "Humidity: " + String(humidity) + " %<br>Temperature: " + String(temperature) + " *C";
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("");
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println(data);
    client.println("<br><a href=\"/relay/on/1min\">Relay ON for 1 min</a>");
    client.println("<br><a href=\"/relay/on/3min\">Relay ON for 3 min</a>");
    client.println("<br><a href=\"/relay/on/5min\">Relay ON for 5 min</a>");
    client.println("<br><a href=\"/relay/on/10min\">Relay ON for 10 min</a>");
    client.println("<br><a href=\"/relay/on/humidity\">Relay ON until 80% humidity</a>");
    client.println("<br><a href=\"/relay/off\">Relay OFF</a>");
    client.println("</html>");
    return;
  } else {
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Type: text/html");
    client.println("");
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("404 Not Found");
    client.println("</html>");
    return;
  }

  // Prepare the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.print("Relay is now: ");
  client.print(relayState ? "ON" : "OFF");
  client.println("<br><a href=\"/relay/on/1min\">Relay ON for 1 min</a>");
  client.println("<br><a href=\"/relay/on/3min\">Relay ON for 3 min</a>");
  client.println("<br><a href=\"/relay/on/5min\">Relay ON for 5 min</a>");
  client.println("<br><a href=\"/relay/on/10min\">Relay ON for 10 min</a>");
  client.println("<br><a href=\"/relay/on/humidity\">Relay ON until 80% humidity</a>");
  client.println("<br><a href=\"/relay/off\">Relay OFF</a>");
  client.println("</html>");
}

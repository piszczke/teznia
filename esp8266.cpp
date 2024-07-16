#include <ESP8266WiFi.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// DHT Sensor settings
#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Relay settings
#define RELAY_PIN D5

// Kill switch settings
#define KILL_SWITCH_PIN D6

// Mode selection switch settings
#define MODE_SWITCH_PIN D7

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "ARRIS-44C9";
const char* password = "7A49DD21AA6C191B";

WiFiServer server(80);

unsigned long relayOffTime = 0;
bool relayState = false;
bool humidityControl = false;
String currentMode = "OFF";
unsigned long timeRemaining = 0;

unsigned long lastModeSwitchPress = 0;
int modeIndex = 0;
const int numModes = 7;
String modes[numModes] = {"OFF", "1 min", "3 min", "5 min", "10 min", "Humidity < 80%", "ON"};

String ipAddress = "";

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Ensure relay is off at startup

  pinMode(KILL_SWITCH_PIN, INPUT_PULLUP);
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);

  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();

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
  ipAddress = WiFi.localIP().toString();
  Serial.println(ipAddress);
}

void loop() {
  // Handle kill switch
  if (digitalRead(KILL_SWITCH_PIN) == LOW) {
    digitalWrite(RELAY_PIN, LOW);
    relayState = false;
    humidityControl = false;
    currentMode = "OFF";
    timeRemaining = 0;
  }

  // Handle mode selection switch
  if (digitalRead(MODE_SWITCH_PIN) == LOW) {
    if (millis() - lastModeSwitchPress > 500) { // Debounce
      modeIndex = (modeIndex + 1) % numModes;
      currentMode = modes[modeIndex];
      lastModeSwitchPress = millis();

      if (currentMode == "1 min") {
        digitalWrite(RELAY_PIN, HIGH);
        relayState = true;
        relayOffTime = millis() + 60000;
      } else if (currentMode == "3 min") {
        digitalWrite(RELAY_PIN, HIGH);
        relayState = true;
        relayOffTime = millis() + 180000;
      } else if (currentMode == "5 min") {
        digitalWrite(RELAY_PIN, HIGH);
        relayState = true;
        relayOffTime = millis() + 300000;
      } else if (currentMode == "10 min") {
        digitalWrite(RELAY_PIN, HIGH);
        relayState = true;
        relayOffTime = millis() + 600000;
      } else if (currentMode == "Humidity < 80%") {
        digitalWrite(RELAY_PIN, HIGH);
        relayState = true;
        humidityControl = true;
      } else if (currentMode == "ON") {
        digitalWrite(RELAY_PIN, HIGH);
        relayState = true;
        humidityControl = false;
        relayOffTime = 0;
      } else {
        digitalWrite(RELAY_PIN, LOW);
        relayState = false;
        humidityControl = false;
        relayOffTime = 0;
      }
    }
  }

  // Handle relay off timing
  if (relayState && millis() > relayOffTime && relayOffTime > 0) {
    digitalWrite(RELAY_PIN, LOW);
    relayState = false;
    humidityControl = false;
    currentMode = "OFF";
    timeRemaining = 0;
  } else if (relayState && relayOffTime > 0) {
    timeRemaining = (relayOffTime - millis()) / 1000;
  }

  // Update sensor readings
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Display sensor readings and relay status on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("TEZNIA systems by M&K");
  display.print("IP: ");
  display.println(ipAddress);
  display.print("Humidity: ");
  display.print(humidity);
  display.println(" %");
  display.print("Temp: ");
  display.print(temperature);
  display.println(" C");
  display.print("Relay: ");
  display.println(relayState ? "ON" : "OFF");
  display.print("Mode: ");
  display.println(currentMode);
  if (relayState && !humidityControl && relayOffTime > 0) {
    display.print("Time: ");
    display.print(timeRemaining);
    display.println(" s");
  }
  display.display();

  // Check if humidity control is active
  if (humidityControl && humidity >= 80.0) {
    digitalWrite(RELAY_PIN, LOW);
    relayState = false;
    humidityControl = false;
    currentMode = "OFF";
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
    currentMode = "1 min";
  } else if (req.indexOf("/relay/on/3min") != -1) {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = true;
    relayOffTime = millis() + 180000;
    currentMode = "3 min";
  } else if (req.indexOf("/relay/on/5min") != -1) {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = true;
    relayOffTime = millis() + 300000;
    currentMode = "5 min";
  } else if (req.indexOf("/relay/on/10min") != -1) {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = true;
    relayOffTime = millis() + 600000;
    currentMode = "10 min";
  } else if (req.indexOf("/relay/on/humidity") != -1) {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = true;
    humidityControl = true;
    currentMode = "Humidity < 80%";
  } else if (req.indexOf("/relay/on/on") != -1) {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = true;
    humidityControl = false;
    relayOffTime = 0;
    currentMode = "ON";
  } else if (req.indexOf("/relay/off") != -1) {
    digitalWrite(RELAY_PIN, LOW);
    relayState = false;
    humidityControl = false;
    currentMode = "OFF";
    timeRemaining = 0;
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
    client.println("<br><a href=\"/relay/on/on\">Relay ON</a>");
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
  client.println("<br><a href=\"/relay/on/on\">Relay ON</a>");
  client.println("<br><a href=\"/relay/off\">Relay OFF</a>");
  client.println("</html>");
}

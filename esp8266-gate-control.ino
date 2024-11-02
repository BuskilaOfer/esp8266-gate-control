#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// ======================= Configuration =======================

// WiFi Credentials
const char* ssid = "********";
const char* password = "*******";

// Server Details
const char* host = "www.oref.org.il";
const int httpsPort = 443;
const char* url = "/warningMessages/alert/Alerts.json";

// LED Pin Definitions
const uint8_t GREEN_LED_PIN = D6;    // GPIO12
const uint8_t BLUE_LED_PIN = D7;     // GPIO13
const uint8_t RED_LED_PIN = D8;      // GPIO15
const uint8_t BUILTIN_LED_PIN = D4;  // GPIO2

// Gate Control Pins
const uint8_t GATE_PIN_1 = D5;  // GPIO14
const uint8_t GATE_PIN_2 = D0;  // GPIO16

// Button Pin
const uint8_t BUTTON_PIN = D2;  // GPIO4

// Search Term (in Hebrew)
const char* searchTerm = "קריית";

// Button Debounce Variables
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;  // in milliseconds

// Fetch Data Variables
unsigned long lastFetchTime = 0;
const unsigned long fetchInterval = 2000;  // 2 seconds

// ======================= Function Prototypes =======================
void logSerial(const String& message);
void turnOffLEDs();
void blinkLED(uint8_t pin, unsigned long delayTime, int times);
void blinkLEDs(uint8_t pin1, uint8_t pin2, unsigned long delayTime, int times);
void setSolidLED(uint8_t pin, bool state);
void openGate(int cycles);
void connectToWiFi();
void handleButtonPress();
void fetchDataAndProcess();

// Sub-methods for openGate
void openTerminal();
void clickOpenGateRapidly();
void waitForDelay(unsigned long duration);
void reActivateGatePin2();
void closeTerminal();

// Optional: Function to set RGB LED color (if applicable)
void setRGBColor(int red, int green, int blue);

// ======================= Setup Function =======================
void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  logSerial("\nStarting NODEMCU ESP8266...");

  // Initialize LED Pins as OUTPUT
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(BUILTIN_LED_PIN, OUTPUT);

  // Initialize Gate Control Pins as OUTPUT
  pinMode(GATE_PIN_1, OUTPUT);
  pinMode(GATE_PIN_2, OUTPUT);

  // Initialize Button Pin as INPUT_PULLUP
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Turn off all LEDs initially
  turnOffLEDs();
  digitalWrite(BUILTIN_LED_PIN, LOW);  // Turn off the built-in LED
  logSerial("All LEDs turned off. Built-in LED is OFF.");

  // Connect to WiFi
  connectToWiFi();
}

// ======================= Main Loop Function =======================
void loop() {
  // Handle Button Press with Debouncing
  handleButtonPress();

  // Check WiFi Connection
  if (WiFi.status() != WL_CONNECTED) {
    logSerial("WiFi not connected! Attempting to reconnect...");
    blinkLED(BLUE_LED_PIN, 500, 3);  // Blink blue LED 3 times
    connectToWiFi();
    // Continue the loop to allow handling button presses
  }

  // Fetch Data from Server and Process periodically
  unsigned long currentTime = millis();
  if (currentTime - lastFetchTime >= fetchInterval) {
    lastFetchTime = currentTime;
    fetchDataAndProcess();
  }

  // Small delay to yield to background processes
  delay(10);
}

// ======================= Helper Functions =======================

// Logging function with timestamp
void logSerial(const String& message) {
  Serial.print("[");
  Serial.print(millis());
  Serial.print(" ms] ");
  Serial.println(message);
}

// Turn off all LEDs
void turnOffLEDs() {
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, LOW);
  logSerial("All LEDs are now OFF.");
}

// Blink a single LED
void blinkLED(uint8_t pin, unsigned long delayTime, int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(delayTime);
    digitalWrite(pin, LOW);
    delay(delayTime);
  }
  logSerial(String("Blinked LED on pin ") + String(pin) + " " + String(times) + " times.");
}

// Blink two LEDs simultaneously
void blinkLEDs(uint8_t pin1, uint8_t pin2, unsigned long delayTime, int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin1, HIGH);
    digitalWrite(pin2, HIGH);
    delay(delayTime);
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
    delay(delayTime);
  }
  logSerial(String("Blinked LEDs on pins ") + String(pin1) + " and " + String(pin2) + " " + String(times) + " times.");
}

// Set an LED to a solid state (ON/OFF)
void setSolidLED(uint8_t pin, bool state) {
  digitalWrite(pin, state ? HIGH : LOW);
}

// Open the gate with specified cycles
void openGate(int cycles) {
  logSerial("Starting gate operation...");

  for (int i = 0; i < cycles; i++) {
    openTerminal();

    clickOpenGateRapidly();

    // Re-activate GATE_PIN_2 multiple times with delays
    for (int j = 0; j < 4; j++) {
      waitForDelay(4900);
      reActivateGatePin2();
    }

    closeTerminal();

    // Delay between cycles
    delay(2500);
  }

  logSerial("Gate operation completed.");
}

// Open the terminal; upon a click, the terminal will be open for 30 seconds
void openTerminal() {
  digitalWrite(GATE_PIN_1, HIGH);
  delay(200);
  digitalWrite(GATE_PIN_1, LOW);
  logSerial("Terminal opened.");
}

// Click open gate rapidly three times
void clickOpenGateRapidly() {
  digitalWrite(GATE_PIN_2, HIGH);
  delay(100);
  digitalWrite(GATE_PIN_2, LOW);
  delay(300);

  digitalWrite(GATE_PIN_2, HIGH);
  delay(150);
  digitalWrite(GATE_PIN_2, LOW);
  delay(350);

  digitalWrite(GATE_PIN_2, HIGH);
  delay(200);
  digitalWrite(GATE_PIN_2, LOW);
  delay(400);

  logSerial("Gate clicked rapidly.");
}

// Wait for specified duration in milliseconds
void waitForDelay(unsigned long duration) {
  logSerial("Waiting for " + String(duration) + " milliseconds...");
  delay(duration);
}

// Re-activate GATE_PIN_2 to keep the gate open
void reActivateGatePin2() {
  digitalWrite(GATE_PIN_2, HIGH);
  delay(150);
  digitalWrite(GATE_PIN_2, LOW);
  logSerial("Re-activated GATE_PIN_2.");
}

// Close the terminal so that in the next loop the terminal will be opened again
void closeTerminal() {
  digitalWrite(GATE_PIN_1, HIGH);
  delay(250);
  digitalWrite(GATE_PIN_1, LOW);
  logSerial("Terminal closed.");
}

// Connect to WiFi with retries
void connectToWiFi() {
  logSerial("Connecting to WiFi: " + String(ssid));
  WiFi.begin(ssid, password);

  // Indicate connection attempt with blue LED blinking
  blinkLED(BLUE_LED_PIN, 250, 6);  // 6 blinks (approx. 3 seconds)

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {  // total wait up to 10 seconds
    delay(500);
    Serial.print(".");  // Keeping this separate for continuity
    // Blink blue LED during connection attempt
    digitalWrite(BLUE_LED_PIN, HIGH);
    delay(100);
    digitalWrite(BLUE_LED_PIN, LOW);
    delay(100);
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    logSerial("Connected to WiFi network.");
    logSerial("IP Address: " + WiFi.localIP().toString());
    // Indicate successful connection with a single blue blink
    blinkLED(BLUE_LED_PIN, 200, 1);
  } else {
    logSerial("Failed to connect to WiFi.");
    // Indicate failure with red LED flashing
    setSolidLED(RED_LED_PIN, true);
    delay(500);
    setSolidLED(RED_LED_PIN, false);
  }
}

// Handle button press with debouncing
void handleButtonPress() {
  int buttonState = digitalRead(BUTTON_PIN);
  unsigned long currentTime = millis();

  if (buttonState != lastButtonState) {
    lastDebounceTime = currentTime;
  }

  if ((currentTime - lastDebounceTime) > debounceDelay) {
    if (buttonState == LOW) {  // Button pressed
      logSerial("Button pressed! Initiating gate opening.");
      openGate(6);
      // Add LED indication for button press
      setSolidLED(GREEN_LED_PIN, true);
      delay(500);
      setSolidLED(GREEN_LED_PIN, false);
    }
  }

  lastButtonState = buttonState;
}

// Fetch data from server and process it
void fetchDataAndProcess() {
  // Indicate start of data fetch with blue LED
  setSolidLED(BLUE_LED_PIN, true);

  logSerial("Connecting to server: " + String(host));

  // Create a secure client
  WiFiClientSecure client;
  client.setInsecure();  // WARNING: Skips SSL certificate verification

  // Attempt to connect to the server
  unsigned long connectStartTime = millis();
  if (!client.connect(host, httpsPort)) {
    logSerial("Connection to server failed!");
    // Indicate connection failure with red LED
    setSolidLED(BLUE_LED_PIN, false);
    setSolidLED(RED_LED_PIN, true);
    delay(500);
    setSolidLED(RED_LED_PIN, false);
    return;
  }
  unsigned long connectEndTime = millis();
  logSerial("Connected to server. Connection time: " + String(connectEndTime - connectStartTime) + " ms");

  // Formulate the GET request
  String getRequest = String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n";

  logSerial("Sending GET request:\n" + getRequest);

  // Send the GET request
  client.print(getRequest);

  // Read the status line
  String statusLine = client.readStringUntil('\n');
  logSerial("Status Line: " + statusLine);

  // Parse the status code
  int statusCode = 0;
  int firstSpace = statusLine.indexOf(' ');
  int secondSpace = statusLine.indexOf(' ', firstSpace + 1);
  if (firstSpace > 0 && secondSpace > firstSpace) {
    String codeStr = statusLine.substring(firstSpace + 1, secondSpace);
    statusCode = codeStr.toInt();
  }

  if (statusCode != 200) {
    logSerial("HTTP request failed with status code: " + String(statusCode));
    setSolidLED(BLUE_LED_PIN, false);  // Turn off blue LED as fetch failed
    setSolidLED(RED_LED_PIN, true);
    delay(500);
    setSolidLED(RED_LED_PIN, false);
    return;
  }

  // Variables to store Content-Length if present
  long contentLength = -1;

  // Read headers
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }

    // Check for Content-Length header
    if (line.startsWith("Content-Length:")) {
      contentLength = line.substring(strlen("Content-Length:")).toInt();
    }
  }

  // Check Content-Length and handle accordingly
  if (contentLength <= 5) {
    logSerial("Content-Length is less than or equal to 5 bytes. Treating as empty response. Skipping processing.");

    // Turn off blue LED as processing is skipped
    setSolidLED(BLUE_LED_PIN, false);

    client.stop();  // Close the connection
    return;
  }
  logSerial("Content-Length: " + String(contentLength) + " bytes");

  // Check if data is available
  if (!client.available()) {
    logSerial("No data received from server.");
    // Indicate no data with red LED blinking
    blinkLED(RED_LED_PIN, 500, 3);

    // Turn off blue LED as fetch failed
    setSolidLED(BLUE_LED_PIN, false);

    client.stop();
    return;
  }

  // Read the JSON payload
  logSerial("Reading JSON payload...");
  unsigned long payloadStartTime = millis();
  String payload = "";
  while (client.available()) {
    char c = client.read();
    payload += c;
  }
  unsigned long payloadEndTime = millis();
  logSerial("JSON Payload Received:");
  logSerial(payload);
  logSerial("Time taken to read payload: " + String(payloadEndTime - payloadStartTime) + " ms");

  // Turn off blue LED as response has been received
  setSolidLED(BLUE_LED_PIN, false);  // Indicate end of data fetch

  client.stop();  // Close the connection

  // Remove BOM if present
  if (payload.length() >= 3) {
    uint8_t firstByte = payload[0];
    uint8_t secondByte = payload[1];
    uint8_t thirdByte = payload[2];
    if (firstByte == 0xEF && secondByte == 0xBB && thirdByte == 0xBF) {
      logSerial("BOM detected. Removing BOM...");
      payload.remove(0, 3);
      logSerial("BOM removed. Updated Payload:");
      logSerial(payload);
    }
  }

  // Check if payload is not empty
  if (payload.length() == 0) {
    logSerial("Payload is empty after BOM removal.");
    // Indicate empty payload
    blinkLED(RED_LED_PIN, 500, 3);
    return;
  }

  // Parse the JSON
  logSerial("Parsing JSON...");

  // Define a capacity based on expected JSON size
  const size_t capacity = JSON_OBJECT_SIZE(5) + JSON_ARRAY_SIZE(1) + 200;
  DynamicJsonDocument doc(capacity);

  unsigned long parseStartTime = millis();
  // Deserialize the JSON
  DeserializationError error = deserializeJson(doc, payload);
  unsigned long parseEndTime = millis();

  if (error) {
    logSerial("JSON parsing failed: " + String(error.c_str()));
    logSerial("Time taken to parse JSON: " + String(parseEndTime - parseStartTime) + " ms");
    // Indicate JSON parsing failure with red LED blinking
    blinkLED(RED_LED_PIN, 500, 3);
    return;
  }

  logSerial("JSON parsing successful.");
  logSerial("Time taken to parse JSON: " + String(parseEndTime - parseStartTime) + " ms");

  // Extract the 'data' array
  JsonArray data = doc["data"];
  if (!data) {
    logSerial("No 'data' field found in JSON.");
    // Indicate missing 'data' field with blue LED blink
    blinkLED(BLUE_LED_PIN, 500, 1);
    return;
  }

  logSerial("'data' array found in JSON.");
  logSerial("Number of items in 'data': " + String(data.size()));

  if (data.size() == 0) {
    logSerial("'data' array is empty.");
    // Indicate empty 'data' array with red LED blinking
    blinkLED(RED_LED_PIN, 500, 3);
    return;
  }

  // Iterate through the 'data' array and search for the term
  bool found = false;
  logSerial("Searching for the term: " + String(searchTerm));
  for (JsonVariant value : data) {
    if (value.is<const char*>()) {
      String location = value.as<const char*>();
      logSerial("Checking location: " + location);
      if (location.indexOf(searchTerm) != -1) {
        found = true;
        logSerial("Search term found in location: " + location);
        break;
      }
    } else {
      logSerial("Non-string value encountered in 'data' array.");
    }
  }

  // Control LEDs and gate based on search results
  if (found) {
    logSerial("Search term '" + String(searchTerm) + "' found.");
    setSolidLED(GREEN_LED_PIN, true);  // Turn on green LED
    logSerial("Green LED is ON.");
    openGate(6);
    setSolidLED(GREEN_LED_PIN, false);  // Turn off green LED
    logSerial("Green LED is OFF.");
  } else {
    logSerial("Search term '" + String(searchTerm) + "' not found.");
    // Blink blue and red LEDs to indicate no match
    blinkLEDs(BLUE_LED_PIN, RED_LED_PIN, 500, 3);
    logSerial("Blue and Red LEDs blinked 3 times.");
  }
}

// Optional: Function to set RGB LED color (if applicable)
void setRGBColor(int red, int green, int blue) {
  digitalWrite(RED_LED_PIN, red ? HIGH : LOW);
  digitalWrite(GREEN_LED_PIN, green ? HIGH : LOW);
  digitalWrite(BLUE_LED_PIN, blue ? HIGH : LOW);
  logSerial("Set RGB LED to R:" + String(red) + " G:" + String(green) + " B:" + String(blue) + ".");
}

#include <SPI.h>
#include <Ethernet.h>
#include <FastLED.h>

// Network configuration
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // MAC address
IPAddress ip(192, 168, 1, 177);                      // Static IP
IPAddress gateway(192, 168, 1, 1);                  // Gateway
IPAddress subnet(255, 255, 255, 0);                 // Subnet mask
unsigned int localPort = 5005;                      // Port to listen on

// Create a server object on port 5005
EthernetServer server(localPort);

// LED configuration
#define NUM_LEDS 26
#define DATA_PIN 6
CRGB leds[NUM_LEDS];

// LED strip ranges
const int stripRanges[5][2] = {
  {0, 5},   // Strip 1 (Red)
  {6, 10},  // Strip 2 (Green)
  {11, 15}, // Strip 3 (Blue)
  {16, 20}, // Strip 4 (Purple)
  {21, 25}  // Strip 5 (Warm White)
};

// Colors for each strip
const CRGB stripColors[5] = {
  CRGB::Red,
  CRGB::Green,
  CRGB::Blue,
  CRGB::Purple,
  CRGB::Wheat
};

void setup() {
  // Initialize Ethernet
  Ethernet.begin(mac, ip, gateway, subnet);

  // Start listening on the specified port
  server.begin();

  // Start serial communication for debugging
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port to connect. Needed for native USB port
  }
  Serial.print("Server is listening on IP: ");
  Serial.println(Ethernet.localIP());

  // Initialize FastLED
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  clearAllStrips();
  FastLED.show();
}

void loop() {
  // Check for incoming client connections
  EthernetClient client = server.available();

  if (client) {
    Serial.println("Client connected.");

    String data = "";

    while (client.connected()) {
      while (client.available() > 0) {
        char c = client.read();
        data += c;
      }

      if (data.length() > 0) {
        Serial.print("Received data: ");
        Serial.println(data);

        // Validate data as a string and split by ';'
        String values[5];
        if (isValidPacket(data) && splitString(data, values, 5)) {
          int stripIndex = values[0].toInt() - 1; // Convert to 0-based index
          String state = values[1];

          if (stripIndex >= 0 && stripIndex < 5) {
            if (state == "on") {
              setStrip(stripIndex, true);
            } else if (state == "off") {
              setStrip(stripIndex, false);
            } else {
              Serial.println("Invalid state. Use 'on' or 'off'.");
            }
          } else {
            Serial.println("Invalid strip index.");
          }
        } else {
          Serial.println("Invalid packet: Incorrect format or number of values.");
        }

        // Clear data for the next packet
        data = "";
      }
    }

    client.stop();
    Serial.println("Client disconnected.");
  }
}

// Function to validate if the packet is a valid string
bool isValidPacket(String data) {
  for (int i = 0; i < data.length(); i++) {
    if (!isPrintable(data[i])) {
      return false;
    }
  }
  return true;
}

// Function to split the string by ';' into an array of Strings
bool splitString(String data, String result[], int expectedCount) {
  int index = 0;
  int start = 0;
  int end = data.indexOf(';');

  while (end != -1) {
    result[index++] = data.substring(start, end);
    start = end + 1;
    end = data.indexOf(';', start);

    if (index > expectedCount) {
      return false; // Too many values
    }
  }

  // Add the last value
  if (index < expectedCount) {
    result[index++] = data.substring(start);
  }

  // Ensure the expected count matches
  return index == expectedCount;
}

// Function to turn a specific strip on or off
void setStrip(int stripIndex, bool state) {
  int start = stripRanges[stripIndex][0];
  int end = stripRanges[stripIndex][1];

  CRGB color = state ? stripColors[stripIndex] : CRGB::Black;
  for (int i = start; i <= end; i++) {
    leds[i] = color;
  }

  FastLED.show();
  Serial.print("Strip ");
  Serial.print(stripIndex + 1);
  Serial.print(" turned ");
  Serial.println(state ? "on" : "off");
}

// Function to clear all strips (turn them off)
void clearAllStrips() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
}

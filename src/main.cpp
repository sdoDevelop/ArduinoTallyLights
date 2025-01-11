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
#define NUM_STRIPS 5
#define NUM_LEDS_PER_STRIP 6

const int dataPins[NUM_STRIPS] = {2, 3, 4, 5, 6}; // Data pins for each strip
CRGB leds[NUM_STRIPS][NUM_LEDS_PER_STRIP];

// Colors for each strip
const CRGB stripColors[NUM_STRIPS] = {
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

  // Initialize FastLED for each strip
  for (int i = 0; i < NUM_STRIPS; i++) {
    FastLED.addLeds<WS2812, dataPins[i], GRB>(leds[i], NUM_LEDS_PER_STRIP);
    clearStrip(i);
  }
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

          if (stripIndex >= 0 && stripIndex < NUM_STRIPS) {
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
  CRGB color = state ? stripColors[stripIndex] : CRGB::Black;
  for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
    leds[stripIndex][i] = color;
  }

  FastLED.show();
  Serial.print("Strip ");
  Serial.print(stripIndex + 1);
  Serial.print(" turned ");
  Serial.println(state ? "on" : "off");
}

// Function to clear a specific strip (turn it off)
void clearStrip(int stripIndex) {
  for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
    leds[stripIndex][i] = CRGB::Black;
  }
}

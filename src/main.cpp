#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>

// Network configuration
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // MAC address
IPAddress ip(192, 168, 168, 10);                      // Static IP
IPAddress gateway(192, 168, 168, 1);                  // Gateway
IPAddress subnet(255, 255, 255, 0);                 // Subnet mask
unsigned int localPort = 5005;                      // Port to listen on

// Create a server object on port 5005
EthernetServer server(localPort);

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
        if (isValidPacket(data)) {
          String values[5];
          if (splitString(data, values, 5)) {
            Serial.println("Valid packet with 5 values:");
            for (int i = 0; i < 5; i++) {
              Serial.print("Value ");
              Serial.print(i + 1);
              Serial.print(": ");
              Serial.println(values[i]);
            }
          } else {
            Serial.println("Invalid packet: Incorrect number of values.");
          }
        } else {
          Serial.println("Invalid packet: Does not meet criteria.");
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

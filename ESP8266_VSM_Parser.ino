#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <espnow.h>

#define MY_NAME "VSM 1"
#define MY_ROLE ESP_NOW_ROLE_CONTROLLER
#define WIFI_CHANNEL 1

#define verbose true

#define BoxName "VSM 1"

const char* ssid = "ESP8266-AP";  // The name of the Wi-Fi network the ESP8266 will create
const char* password = "12345";   // Password for the Wi-Fi network

WiFiUDP udp;
unsigned int localPort = 50101;  // Port to listen for UDP data

// Broadcast MAC address for ESP-NOW
uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

// ESP-NOW communication packet
typedef struct struct_message {
  uint8_t msgType;     // Type of message
  uint8_t macAddr[6];  // MAC address of the message sender
  unsigned int Right_Score;
  unsigned int Left_Score;
  unsigned int Seconds_Remaining;
  unsigned int Minutes_Remaining;
  bool Green_Light;
  bool Red_Light;
  bool White_Green_Light;
  bool White_Red_Light;
  bool Yellow_Green_Light;
  bool Yellow_Red_Light;
  bool Yellow_Card_Green;
  bool Yellow_Card_Red;
  bool Red_Card_Green;
  bool Red_Card_Red;
  bool Priority_Left;
  bool Priority_Right;
  char customMessage[32];  // Array to hold the name of the sender
} struct_message;

// Initializes the packet
struct_message packet;
struct_message packet_prev;  // Previous packet to detect changes
bool new_data = false;

// Timer variables for periodic broadcasting
unsigned long lastTransmitTime = 0;
unsigned long transmitInterval = 1000;  // Interval to send data (in milliseconds)

// Function to compare two packets
bool areDataPacketsIdentical(const struct_message& packet1, const struct_message& packet2) {
  return memcmp(&packet1, &packet2, sizeof(struct_message)) == 0;
}

// Parse different types of packets (time, lights, score)
void parseTimePacket(char* packetBuffer) {
  packet.Minutes_Remaining = (packetBuffer[5] - '0');
  packet.Seconds_Remaining = (packetBuffer[7] - '0') * 10 + (packetBuffer[8] - '0');
}

void parseLightsPacket(char* packetBuffer) {
  packet.Red_Light = (packetBuffer[3] - '0') == 1;
  packet.Green_Light = (packetBuffer[5] - '0') == 1;
  packet.White_Red_Light = (packetBuffer[7] - '0') == 1;
  packet.White_Green_Light = (packetBuffer[9] - '0') == 1;
}

void parseScorePacket(char* packetBuffer) {
  packet.Right_Score = (packetBuffer[4] - '0') * 10 + (packetBuffer[5] - '0');
  packet.Left_Score = (packetBuffer[7] - '0') * 10 + (packetBuffer[8] - '0');
  packet.Yellow_Card_Green = (packetBuffer[11] - '0') == 1;
  packet.Red_Card_Green = (packetBuffer[13] - '0') == 1;
  packet.Yellow_Card_Red = (packetBuffer[17] - '0') == 1;
  packet.Red_Card_Red = (packetBuffer[19] - '0') == 1;

  int priority = atoi(&packetBuffer[25]);
  packet.Priority_Left = (priority == 2);
  packet.Priority_Right = (priority == 1);
}

// Callback for ESP-NOW transmission
void transmissionComplete(uint8_t* receiver_mac, uint8_t transmissionStatus) {
  if (transmissionStatus == 0) {
    Serial.println("Data sent successfully");
  } else {
    Serial.print("Error code: ");
    Serial.println(transmissionStatus);
  }
}

void setup() {
  Serial.begin(115200);

  // Set Wi-Fi mode to AP+STA (access point and station)
  WiFi.mode(WIFI_AP_STA);

  // Set up the ESP8266 as an Access Point
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point IP: ");
  Serial.println(IP);

  // Start listening for UDP packets
  udp.begin(localPort);
  Serial.printf("Listening for UDP on port %d\n", localPort);

  // Initialize ESP-NOW
  WiFi.disconnect();
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW initialization failed");
    return;
  }
  esp_now_set_self_role(MY_ROLE);
  esp_now_register_send_cb(transmissionComplete);

  // Use strcpy to copy "VSM 1" into the customMessage array
  strcpy(packet.customMessage, BoxName);
}

void loop() {
  // Check if there is new UDP data
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char packetBuffer[255];
    int len = udp.read(packetBuffer, 255);
    packetBuffer[len] = 0;  // Null-terminate the string

    // Identify packet type based on size
    if (packetSize == 10) {
      parseTimePacket(packetBuffer);
    } else if (packetSize == 11) {
      parseLightsPacket(packetBuffer);
    } else if (packetSize == 29) {
      parseScorePacket(packetBuffer);
    }

    // Set additional fields in the packet
    packet.msgType = 1;               // Example type
    WiFi.macAddress(packet.macAddr);  // Set the sender's MAC address
    strncpy(packet.customMessage, MY_NAME, sizeof(packet.customMessage) - 1);
    packet.customMessage[sizeof(packet.customMessage) - 1] = '\0';  // Ensure null-termination

    // Set new_data flag if the data has changed
    if (!areDataPacketsIdentical(packet, packet_prev)) {
      packet_prev = packet;  // Update previous packet
      new_data = true;
    }
  }

  // Send new data if updated or periodically
  if (new_data || (millis() - lastTransmitTime > transmitInterval)) {
    lastTransmitTime = millis();
    esp_now_send(broadcastAddress, (uint8_t*)&packet, sizeof(packet));  // Broadcast to all devices
    Serial.println("Broadcasting Data via ESP-NOW.");
    new_data = false;
  }
}

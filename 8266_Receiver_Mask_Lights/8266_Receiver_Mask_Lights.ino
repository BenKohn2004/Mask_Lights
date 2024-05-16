#include <ESP8266WiFi.h>
#include <espnow.h>

// OLED 1306 Libraries
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <cmath>  // Include the cmath library for ceil function

// For the Matrix LEDs
#include <Adafruit_NeoPixel.h>

#include <map>

// Define a struct for your message data
struct struct_message {
  uint8_t msgType;     // Transmitter, Receiver
  uint8_t macAddr[6];  // The MAC address of the message sender
  int unsigned Right_Score;
  int unsigned Left_Score;
  int unsigned Seconds_Remaining;
  int unsigned Minutes_Remaining;
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
  char customMessage[32];  // Array to hold a fixed-length string of 31 characters + null terminator, This is the name of the sender
};

struct_message incomingReadings;
struct_message msg;  // Creates a global variable to hold messages

std::map<String, struct_message> messageMap;

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels

#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Button Variables for Pairing Button
#define BUTTON_PIN 12

// Define button pin and LED pins
// #define BUTTON_PIN_RED_GREEN 16  // Green Red Select Button, uses an internal PULLDOWN resistor
#define Green_PIN 0  // D3 Green LED
#define Red_PIN 2    // D4 Red LED
#define Blue_PIN 14  // Paired

#define brightness 20  // Sets the brightness level

// Define the data pin connected to the LED matrices
#define LED_PIN 13
#define NUM_LEDS_PER_MATRIX 64
#define NUM_MATRICES 2

// Define variables for pairing button handling
bool buttonState = HIGH;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// Define an iterator and it variables
unsigned int iterator = 0;
unsigned int it = 0;

// New Data Flag
bool new_data = false;

// Define a simple structure to represent RGB colors
struct RGBColor {
  uint8_t r;  // Red component
  uint8_t g;  // Green component
  uint8_t b;  // Blue component
};

// Create a struct_message called myData
struct_message myData;

int currentIndex = 0;  // Variable to keep track of the current position in messageList
// std::vector<struct_message> messageList;  // Define a vector of pairs to store incoming messages

Adafruit_NeoPixel matrix[NUM_MATRICES];
bool allLedsOn = false;

void parseMacAddress(const String &macAddressStr, uint8_t macAddr[6]) {
  char macChars[18];
  macAddressStr.toCharArray(macChars, sizeof(macChars));
  sscanf(macChars, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
         &macAddr[0], &macAddr[1], &macAddr[2],
         &macAddr[3], &macAddr[4], &macAddr[5]);
}


void printMAC(const uint8_t *mac_addr) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
}

String macToString(uint8_t *mac) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(macStr);
}

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  // Convert the MAC address to a String
  String macAddress = macToString(mac);

  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Size of message: ");
  Serial.print(len);
  Serial.print(" Data bytes received from: ");
  printMAC(mac);
  Serial.println();

  // Store the received data using the MAC address as the key
  messageMap[macAddress] = incomingReadings;
  new_data = true;  // Updates the system if new data is received
}

void changeLEDColor(RGBColor color) {
  for (int i = 0; i < NUM_MATRICES; i++) {
    for (int j = 0; j < NUM_LEDS_PER_MATRIX; j++) {
      matrix[i].setPixelColor(j, matrix[i].Color(color.r, color.g, color.b));  // Set LED color
    }
    matrix[i].show();  // Display the color on the matrix
  }
}

void printMessageMap() {
  // Iterate over the messageMap using an iterator
  for (auto it = messageMap.begin(); it != messageMap.end(); ++it) {
    // Get the MAC address (key)
    String macAddress = it->first;

    // Get the struct_message (value)
    struct_message message = it->second;

    // Print the MAC address and customMessage
    Serial.print("MAC Address: ");
    Serial.println(macAddress);
    Serial.print("Custom Message: ");
    Serial.println(message.customMessage);
    Serial.println();  // Add a blank line for clarity
  }
}

struct_message getMessageAtIndex(int index) {
  int currentIndex = 0;

  // Iterate over the messageMap using a ranged-based for loop
  for (const auto &pair : messageMap) {
    // If the current index matches the input index, return the struct_message
    if (currentIndex == index) {
      Serial.print("Name: ");
      Serial.println(pair.second.customMessage);
      return pair.second;
    }

    // Move to the next index
    ++currentIndex;
  }

  // If the index is out of bounds, return a default struct_message
  struct_message defaultMessage;
  return defaultMessage;
}

void setup() {
  // Init Serial Monitor
  Serial.begin(2400);

  // Initialize the Pairing button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize the LED pin as an output
  pinMode(Green_PIN, OUTPUT);
  pinMode(Red_PIN, OUTPUT);
  pinMode(Blue_PIN, OUTPUT);

  for (int i = 0; i < NUM_MATRICES; i++) {
    matrix[i] = Adafruit_NeoPixel(NUM_LEDS_PER_MATRIX, LED_PIN, NEO_GRB + NEO_KHZ800);
    matrix[i].begin();
    matrix[i].setBrightness(2);  // Set brightness to half (0-255)
    matrix[i].show();            // Initialize all LEDs to 'off'
  }

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());
  WiFi.disconnect();

  // Get MAC address as a string
  String macAddressStr = WiFi.macAddress();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);

  delay(2000);  // Delay to allow startup
  Serial.println("Setup Complete");
}

void loop() {
  // Check if the button state has changed
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // Check for button debouncing
  if (millis() - lastDebounceTime > debounceDelay) {

    if (reading != buttonState) {
      buttonState = reading;

      // If the button is pressed, increment the iterator and print it
      if (buttonState == LOW) {
        iterator = (iterator + 1) % ((messageMap.size() * 2) + 1);  // Ensure the iterator is not larger than 2 times the messageMap plus 1
        new_data = true;
        Serial.println("Button Pressed");
        Serial.print("The iterator is: ");
        Serial.println(iterator);
      }
    }
  }

  // it is the iterator for each box green and red, iterator is each box twice, plus the select box
  it = ceil(iterator / 2.0);  // Use ceil function to round up the result of division

  // Update lastButtonState
  lastButtonState = reading;

  if (new_data == true) {
    display.clearDisplay();
    display.setCursor(0, 0);              // Start at top-left corner
    display.setTextSize(1);               // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);  // Draw white text
    display.println();
    display.println();

    msg = getMessageAtIndex(it - 1);
    if (iterator == 0) {
      printMessageMap();
      display.println("Select Box");
      // analogWrite(Green_PIN, 0);
      // analogWrite(Red_PIN, 0);
      digitalWrite(Green_PIN, 0);
      digitalWrite(Red_PIN, 0);
      changeLEDColor({ 0, 0, 0 });
    } else if (iterator % 2 == 1) {
      display.println(msg.customMessage);
      Serial.print(msg.customMessage);
      display.println("Green");
      Serial.print(" Green. ");
      digitalWrite(Green_PIN, 1);
      digitalWrite(Red_PIN, 0);
      if (msg.Green_Light == 1) {
        changeLEDColor({ 0, 255, 0 });  // Green
        Serial.println("Green Light Lit");
      } else {
        changeLEDColor({ 0, 0, 0 });  // Black
        Serial.println("Green Light Extinguished");
      }
    } else if (iterator % 2 == 0) {
      display.println(msg.customMessage);
      Serial.print(msg.customMessage);
      display.println("Red");
      Serial.print(" Red. ");
      digitalWrite(Green_PIN, 0);
      digitalWrite(Red_PIN, 1);
      if (msg.Red_Light == 1) {
        changeLEDColor({ 255, 0, 0 });  // Red
        Serial.println("Red Light Lit");
      } else {
        changeLEDColor({ 0, 0, 0 });  // Black
        Serial.println("Red Light Extinguished");
      }
    }
    display.display();
    new_data = false;
    Serial.print("it is: ");
    Serial.print(it);
    Serial.print(" and the iterator is: ");
    Serial.print(iterator);
    Serial.println(". ");
    Serial.println("New_Data set to False.");
  }
}

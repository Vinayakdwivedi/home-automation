// Uncomment the following line to enable serial debug output
//#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
       #define DEBUG_ESP_PORT Serial
       #define NODEBUG_WEBSOCKETS
       #define NDEBUG
#endif 

#include <Arduino.h>
#include <WiFi.h>
#include "SinricPro.h"
#include "SinricProSwitch.h"

#include <map>

#define WIFI_SSID         "bappahive"    
#define WIFI_PASS         "bapparawal"
#define APP_KEY           "298571a2-3672-4cb9-9db2-0ee9f6972b1b"
#define APP_SECRET        "f7037edf-9f14-4dd3-9d58-b63e7dc0bcc7-0545b589-4ee3-4c41-a204-8463d48ed7c7"

// Device IDs
#define device_ID_1   "6738d519112bc57a500a04b8"
#define device_ID_2   "6738d7301eb8df266072f5b0"
#define device_ID_3   "6738d6a9112bc57a500a05af"
#define device_ID_4   "SWITCH_ID_NO_4_HERE"

// GPIO pins for Relays, Switches, and Motion Sensor
#define RelayPin1 23  // D23
#define RelayPin2 22  // D22
#define RelayPin3 21  // D21
#define RelayPin4 19  // D19

#define SwitchPin1 13  // D13
#define SwitchPin2 12  // D12
#define SwitchPin3 14  // D14
#define SwitchPin4 27  // D27

#define MotionSensorPin 18  // D18 (Connect to PIR motion sensor)

#define wifiLed   2   // D2

#define BAUD_RATE   9600

// Timer variables
unsigned long motionStopTime = 0; // Time when motion stops
bool motionActive = false;        // Indicates if motion is currently active
const unsigned long delayTime = 0.25 * 60 * 1000; // 15 minutes in milliseconds

typedef struct {
  int relayPIN;
  int flipSwitchPIN;
} deviceConfig_t;

// Map for devices
std::map<String, deviceConfig_t> devices = {
    {device_ID_1, {RelayPin1, SwitchPin1}},
    {device_ID_2, {RelayPin2, SwitchPin2}},
    {device_ID_3, {RelayPin3, SwitchPin3}},
    {device_ID_4, {RelayPin4, SwitchPin4}}     
};

bool onPowerState(String deviceId, bool &state) {
  Serial.printf("%s: %s\r\n", deviceId.c_str(), state ? "on" : "off");
  int relayPIN = devices[deviceId].relayPIN; // Get relay pin for corresponding device
  digitalWrite(relayPIN, !state);            // Set relay state (active LOW)
  return true;
}

void setupRelays() {
  for (auto &device : devices) {
    int relayPIN = device.second.relayPIN;
    pinMode(relayPIN, OUTPUT);
    digitalWrite(relayPIN, HIGH); // Relay off by default (HIGH = off for active LOW relays)
  }
}

void setupWiFi() {
  Serial.printf("\r\n[WiFi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  digitalWrite(wifiLed, HIGH);
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

void setupSinricPro() {
  for (auto &device : devices) {
    const char *deviceId = device.first.c_str();
    SinricProSwitch &mySwitch = SinricPro[deviceId];
    mySwitch.onPowerState(onPowerState);
  }

  SinricPro.begin(APP_KEY, APP_SECRET);
  SinricPro.restoreDeviceStates(true);
}

void setupMotionSensor() {
  pinMode(MotionSensorPin, INPUT); // Set motion sensor pin as input
}

void checkMotionSensor() {
  static bool lastMotionState = LOW; // Initialize to LOW (no motion)
  bool motionDetected = digitalRead(MotionSensorPin); // Read motion sensor state

  // When motion is detected
  if (motionDetected && !motionActive) {
    Serial.println("[Motion Sensor]: Motion Detected!");
    digitalWrite(RelayPin2, LOW); // Turn on Relay 2 (active LOW)

    // Send event to SinricPro
    SinricProSwitch &mySwitch = SinricPro[device_ID_2];
    mySwitch.sendPowerStateEvent(true);

    motionActive = true; // Mark motion as active
  }

  // When no motion is detected
  if (!motionDetected && motionActive) {
    Serial.println("[Motion Sensor]: No Motion Detected!");
    motionStopTime = millis(); // Record the time when motion stops
    motionActive = false; // Mark motion as inactive
  }

  // Check if 15 minutes have passed since motion stopped
  if (!motionActive && motionStopTime > 0 && millis() - motionStopTime >= delayTime) {
    Serial.println("[Motion Sensor]: Turning off Relay after 15 minutes.");
    digitalWrite(RelayPin2, HIGH); // Turn off Relay 2 (active LOW)

    // Send event to SinricPro
    SinricProSwitch &mySwitch = SinricPro[device_ID_2];
    mySwitch.sendPowerStateEvent(false);

    motionStopTime = 0; // Reset motion stop time
  }
}

void setup() {
  Serial.begin(BAUD_RATE);

  pinMode(wifiLed, OUTPUT);
  digitalWrite(wifiLed, LOW);

  setupRelays();
  setupWiFi();
  setupSinricPro();
  setupMotionSensor();
}

void loop() {
  SinricPro.handle();
  checkMotionSensor();
}

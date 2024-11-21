#include <WiFi.h>
#include <HTTPClient.h>

// Wi-Fi Credentials
const char* ssid = "bappahive";
const char* password = "bapparawal";

// Motion Sensor Pin
#define MotionSensorPin 18

// Server URL (Replace with your laptop's IP and Flask server port, e.g., 5000)
const char* serverURL = "http://127.0.0.1:5000/motion";

void setup() {
  Serial.begin(115200);

  // Initialize the motion sensor pin
  pinMode(MotionSensorPin, INPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void sendMotionDetected() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Make a request to the server
    http.begin(serverURL);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.printf("Motion detected. Server response: %d\n", httpResponseCode);
    } else {
      Serial.printf("Error sending motion signal. HTTP error: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();
  } else {
    Serial.println("Wi-Fi disconnected. Reconnecting...");
    WiFi.begin(ssid, password);
  }
}

void loop() {
  static bool motionActive = false;
  bool motionDetected = digitalRead(MotionSensorPin);

  // Detect rising edge (motion start)
  if (motionDetected && !motionActive) {
    Serial.println("Motion detected!");
    sendMotionDetected();
    motionActive = true;
  }

  // Detect falling edge (motion stop)
  if (!motionDetected && motionActive) {
    Serial.println("Motion stopped.");
    motionActive = false;
  }

  delay(100); // Debounce delay
}

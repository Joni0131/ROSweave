#include "main.h"

Configuration *config;
MyWebServer *webServer;
#include <functional>

std::function<void()> loopFunction = nullptr; // Function object for the loop behavior

void handleWebServer() {
  if (webServer) {
    webServer->handle();
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting up...");
  pinMode(MODE_PIN, INPUT); // Set the mode pin as input with pull-up resistor
  delay(1000); // Wait for a second to stabilize the input
  Serial.println("Checking mode pin...");
  if (digitalRead(MODE_PIN)) {
    // Boot into configuration mode
    Serial.println("Booting into configuration mode...");
    webServer = new MyWebServer();
    webServer->setup();
    loopFunction = handleWebServer; 
  } else {
    // Boot into operational mode
    config = new Configuration(NODE_NAME, SPINTIME_MS, MAX_PUBLISHER, MAX_SERVICES, MAX_SUBSCRIBER);
    Serial.println("Booting into operational mode...");
    delay(2000);
    config->setupHW();
    loopFunction = []() { config->spin(); }; // Assign operational mode to loopFunction
  }
}

void loop() {
  loopFunction(); // Call the assigned function
}
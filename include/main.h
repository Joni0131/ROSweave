#ifndef MAIN_H
#define MAIN_H

#define MAX_PUBLISHER 7
#define MAX_SUBSCRIBER 5
#define MAX_SERVICES 2
#define SPINTIME_MS 100
#define NODE_NAME "carcore"
#define MODE_PIN 15

#include <Arduino.h>

#include "Configuration.h"
#include "MyWebServer.h"

void setup();

void loop();

#endif
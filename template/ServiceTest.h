#ifndef SERVICETEST_H
#define SERVICETEST_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include <vector>
#include <inttypes.h>

#include "GeneralTopic.h"
#include "MicroRosServices.h"

#include "custom_interfaces/srv/service_test.h"

class ServiceTest
{
private:
    /* data */
    MicroRosServices *services;
    GeneralTopic *serviceTopic;
    void registerCallbacks();
public:
    ServiceTest(GeneralTopic *serviceTopic, MicroRosServices *services);
    ~ServiceTest();

    void initTopics();
    void serviceCallback(const void * request, const void * responds);
};

void SERVICETEST_targetCallbackWrapper(const void *request, void *responds, void *serviceTest);

#endif
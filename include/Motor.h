#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include <vector>

#include "GeneralTopic.h"
#include "PinConfiguration.h"
#include "MicroRosPublishers.h"
#include "MicroRosSubscribers.h"
#include "MicroRosServices.h"


class Motor
{
protected:
    /* data */
    std::vector<PinConfiguration> *pin_config;
    MicroRosPublishers *publishers;
    MicroRosSubscribers *subscribers;
    MicroRosServices *services;
    GeneralTopic *targetTopic;
    GeneralTopic *infoTopic;

    void configurePins();

public:
    Motor(std::vector<PinConfiguration> *pin_config, GeneralTopic *targetTopic, GeneralTopic *infoTopic, MicroRosPublishers *publishers, MicroRosSubscribers *subscribers, MicroRosServices *services); 
    ~Motor();
};

#endif
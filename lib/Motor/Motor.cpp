#include "Motor.h"

Motor::Motor(std::vector<PinConfiguration> *pin_config, GeneralTopic *targetTopic, GeneralTopic *infoTopic, MicroRosPublishers *publishers, MicroRosSubscribers *subscribers, MicroRosServices *services)
{
    this->pin_config = pin_config;
    this->publishers = publishers;
    this->subscribers = subscribers;
    this->services = services;
    this->targetTopic = targetTopic;
    this->infoTopic = infoTopic;

    configurePins();
}

Motor::~Motor()
{
}

void Motor::configurePins()
{
    for (int i = 0; i < this->pin_config->size(); i++)
    {
        pinMode(this->pin_config->at(i).pin, this->pin_config->at(i).mode);
        digitalWrite(this->pin_config->at(i).pin, this->pin_config->at(i).value);
    }
}
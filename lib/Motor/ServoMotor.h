#ifndef SERVOMOTOR_H
#define SERVOMOTOR_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>
#include <map>
#include <string>
#include <vector>

#include "Motor.h"
#include "custom_interfaces/msg/servo_motor.h"

enum ServoMotorType
{
    SG90_180,
    SG90_360
};

class ServoMotor : public Motor
{
private:
    /* data */
    Servo myServo;
    ServoMotorType type;
    int min_angle = 0;
    int max_angle = 180;
    void registerCallbacks();
public:
    
    struct ServoMotorFactoryResult {
        ServoMotor* servo;
        GeneralTopic* targetTopic;
        GeneralTopic* infoTopic;
    }; 

    ServoMotor(std::vector<PinConfiguration> *pin_config, GeneralTopic *targetTopic, GeneralTopic *infoTopic, MicroRosPublishers *publishers, MicroRosSubscribers *subscribers, MicroRosServices *services, ServoMotorType type);
    ~ServoMotor();

    void initTopics();
    void targetCallback(const void *msgin);
    void infoCallback(rcl_timer_t *timer, int64_t last_call_time);

    static ServoMotorFactoryResult createFromJsonServo(
        const char* name,
        JsonObject& component,
        MicroRosPublishers* publishers,
        MicroRosSubscribers* subscribers,
        MicroRosServices* services
    );
};

void SERVOMOTOR_targetCallbackWrapper(const void *msgin, void *motor);
#endif
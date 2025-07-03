#include "ServoMotor.h"
#include <functional>
#include <iostream>

ServoMotor::ServoMotor(std::vector<PinConfiguration> *pin_config, GeneralTopic *targetTopic, GeneralTopic *infoTopic, MicroRosPublishers *publishers, MicroRosSubscribers *subscribers, MicroRosServices *services, ServoMotorType type) : Motor(pin_config, targetTopic, infoTopic, publishers, subscribers, services)
{
    this->type = type;
    this->myServo.setPeriodHertz(50);
    this->myServo.attach(this->pin_config->at(0).pin);

    if(type == SG90_180){
        this->min_angle = 0;
        this->max_angle = 180;
    } else if(type == SG90_360){
        this->min_angle = 0;
        this->max_angle = 360;
    }

    this->initTopics();
    this->registerCallbacks();
}

ServoMotor::~ServoMotor()
{
    this->myServo.detach();
}

void ServoMotor::initTopics(){
    custom_interfaces__msg__ServoMotor *infoTopic;
    custom_interfaces__msg__ServoMotor *targetTopic;

    infoTopic = (custom_interfaces__msg__ServoMotor *)this->targetTopic->getMsg();
    targetTopic = (custom_interfaces__msg__ServoMotor *)this->infoTopic->getMsg();

    infoTopic->angle = 0;
    targetTopic->angle = 0;

}

void ServoMotor::registerCallbacks()
{
    Serial.println("[ServoMotor] Registering publisher and subscriber callbacks");
    this->publishers->registerPublisher(this->infoTopic,[this](rcl_timer_t *timer, uint64_t currentTime) {
        this->infoCallback(timer,currentTime);
    });
    Serial.println("[ServoMotor] Registering subscriber callback");
    this->subscribers->registerSubscriber(this->targetTopic, ON_NEW_DATA, SERVOMOTOR_targetCallbackWrapper, this);
    Serial.println("[ServoMotor] Subscriber callback registered");
}

void ServoMotor::targetCallback(const void *msgin)
{
    //Debug this
    custom_interfaces__msg__ServoMotor msg = *((custom_interfaces__msg__ServoMotor *)msgin);
    if (msg.angle >= this->min_angle && msg.angle <= this->max_angle) {
        // update target topic
        custom_interfaces__msg__ServoMotor* targetMsg = ((custom_interfaces__msg__ServoMotor *)this->targetTopic->getMsg());
        targetMsg->header.stamp.sec = msg.header.stamp.sec;
        targetMsg->header.stamp.nanosec = msg.header.stamp.nanosec;
        targetMsg->angle = msg.angle;
        // Set the servo angle
        this->myServo.write(msg.angle);
    } else {
        Serial.printf("[ServoMotor] Invalid angle: %d\n", msg.angle);
    }
}

void ServoMotor::infoCallback(rcl_timer_t *timer, int64_t last_call_time)
{
    // Get corrected epoch time 
    int64_t time_ns = rmw_uros_epoch_nanos();
    custom_interfaces__msg__ServoMotor *msg = (custom_interfaces__msg__ServoMotor *)this->infoTopic->getMsg();
    msg->header.stamp.sec = (int32_t)(time_ns / 1000000000);
    msg->header.stamp.nanosec = (uint32_t)(time_ns % 1000000000);
    msg->angle = (int32_t)this->myServo.read();
    // No need to call setMsg if getMsg() returns the buffer used for publishing
    RCSOFTCHECK(rcl_publish(this->publishers->getPublisher(this->infoTopic->getID()), msg, NULL));
}

ServoMotor::ServoMotorFactoryResult ServoMotor::createFromJsonServo(const char *name, JsonObject &component, MicroRosPublishers *publishers, MicroRosSubscribers *subscribers, MicroRosServices *services)
{
    ServoMotorFactoryResult result = {nullptr, nullptr, nullptr};

    Serial.printf("[createComponentServo] Called for name: %s\n", name);
    std::string targetTopicName = std::string(name) + "_target";
    std::string infoTopicName = std::string(name) + "_info";
    Serial.printf("[createComponentServo] targetTopicName: %s, infoTopicName: %s\n", targetTopicName.c_str(), infoTopicName.c_str());

    // Extract the update interval
    int updateInterval = component["config"]["topicUpdateInterval"].as<int>();
    Serial.printf("[createComponentServo] updateInterval: %d\n", updateInterval);

    // Initialize topics
    Serial.println("[createComponentServo] Initializing topics...");
    result.targetTopic = new GeneralTopic(
        const_cast<char *>(targetTopicName.c_str()), targetTopicName.size() + 1, sizeof(custom_interfaces__msg__ServoMotor), updateInterval, *rosidl_typesupport_c__get_message_type_support_handle__custom_interfaces__msg__ServoMotor());
    result.infoTopic = new GeneralTopic(
        const_cast<char *>(infoTopicName.c_str()), infoTopicName.size() + 1, sizeof(custom_interfaces__msg__ServoMotor), updateInterval, *rosidl_typesupport_c__get_message_type_support_handle__custom_interfaces__msg__ServoMotor());
    Serial.println("[createComponentServo] Topics initialized.");

    // Get the motor type
    const char *motorTypeStr = component["config"]["type"];
    Serial.printf("[createComponentServo] motorTypeStr: %s\n", motorTypeStr);
    ServoMotorType motorType;
    if (strcmp(motorTypeStr, "SG90_180") == 0)
    {
        motorType = SG90_180;
        Serial.println("[createComponentServo] Motor type: SG90_180");
    }
    else if (strcmp(motorTypeStr, "SG90_360") == 0)
    {
        motorType = SG90_360;
        Serial.println("[createComponentServo] Motor type: SG90_360");
    }
    else
    {
        Serial.printf("[createComponentServo] Unknown motor type: %s\n", motorTypeStr);
        return result;
    }

    // Extract pin configuration
    JsonArray pinArray = component["pins"];
    Serial.printf("[createComponentServo] pinArray size: %d\n", pinArray.size());
    std::vector<PinConfiguration> pin_config;
    for (int i = 0; i < pinArray.size(); i++)
    {
        PinConfiguration pin;
        pin.pin = pinArray[i]["number"];
        pin.mode = 0;
        pin.value = 0;
        pin_config.push_back(pin);
        Serial.printf("[createComponentServo] Pin %d: pin=%d, mode=%d, value=%d\n", i, pin.pin, pin.mode, pin.value);
    }

    // Create and initialize the ServoMotor
    Serial.println("[createComponentServo] Creating ServoMotor instance...");
    result.servo = new ServoMotor(&pin_config, result.targetTopic, result.infoTopic, publishers, subscribers, services, motorType);
    Serial.printf("[createComponentServo] ServoMotor '%s' initialized.\n", name);

    return result;
}

void SERVOMOTOR_targetCallbackWrapper(const void *msgin, void *motor)
{
    ((ServoMotor *)motor)->targetCallback(msgin);
}
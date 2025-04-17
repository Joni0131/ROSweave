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
    custom_interfaces__msg__ServoMotor infoTopic;
    custom_interfaces__msg__ServoMotor targetTopic;

    infoTopic = *((custom_interfaces__msg__ServoMotor *)this->targetTopic->getMsg());
    targetTopic = *((custom_interfaces__msg__ServoMotor *)this->infoTopic->getMsg());

    infoTopic.angle = 0;
    targetTopic.angle = 0;

    this->infoTopic->setMsg(&infoTopic);
    this->targetTopic->setMsg(&targetTopic);
}

void ServoMotor::registerCallbacks()
{
    this->publishers->registerPublisher(this->infoTopic,[this](rcl_timer_t *timer, uint64_t currentTime) {this->infoCallback(timer,currentTime);});
    this->subscribers->registerSubscriber(this->targetTopic, ON_NEW_DATA, SERVOMOTOR_targetCallbackWrapper, this);
}

void ServoMotor::targetCallback(const void *msgin)
{
    // Implement the target callback logic here
    custom_interfaces__msg__ServoMotor msg = *((custom_interfaces__msg__ServoMotor *)msgin);
    if (msg.angle >= this->min_angle && msg.angle <= this->max_angle) {
        // update target topic
        custom_interfaces__msg__ServoMotor targetMsg = *((custom_interfaces__msg__ServoMotor *)this->targetTopic->getMsg());
        targetMsg.header.stamp.sec = msg.header.stamp.sec;
        targetMsg.header.stamp.nanosec = msg.header.stamp.nanosec;
        targetMsg.angle = msg.angle;
        this->targetTopic->setMsg(&targetMsg);
        // Set the servo angle
        this->myServo.write(msg.angle);
    } else {
        std::cerr << "Invalid angle: " << msg.angle << std::endl;
    }
}

void ServoMotor::infoCallback(rcl_timer_t *timer, int64_t last_call_time)
{
    // Get corrected epoch time 
    int64_t time_ns = rmw_uros_epoch_nanos();
    custom_interfaces__msg__ServoMotor msg = *((custom_interfaces__msg__ServoMotor *)this->infoTopic->getMsg());
    msg.header.stamp.sec = (int32_t)(time_ns / 1000000000);
    msg.header.stamp.nanosec = (uint32_t)(time_ns % 1000000000);
    msg.angle = this->myServo.read();
    this->infoTopic->setMsg(&msg);
    RCSOFTCHECK(rcl_publish(this->publishers->getPublisher(this->infoTopic->getID()), this->infoTopic->getMsg(), NULL));
}

void SERVOMOTOR_targetCallbackWrapper(const void *msgin, void *motor)
{
    ((ServoMotor *)motor)->targetCallback(msgin);
}

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <Arduino.h>
#include <micro_ros_platformio.h>
#include <ArduinoJson.h>
#include <map>
#include <string>

#include "ServoMotor.h"
#include "HC-SR04.h"

#include "GeneralRosDefinitions.h"
#include "GeneralTopic.h"
#include "MicroRosPublishers.h"
#include "MicroRosServices.h"
#include "MicroRosSubscribers.h"
#include "SPIFFS.h"

class Configuration
{
private:
    /* data */
    rcl_node_t node;
    rclc_support_t support;
    rcl_allocator_t allocator;
    int spintime_ms;
    int max_publishers;
    int max_services;
    int max_subscribers;
    const char* node_name;
    MicroRosPublishers *publishers;
    MicroRosServices *services;
    MicroRosSubscribers *subscribers;

    // Container to store component instances
    std::map<std::string, void *> components;

    // Container to store topics
    std::map<std::string, GeneralTopic *> topics;

    void createNode(const char *node_name);
    void createSupport();
    void createAllocator();
    void createPublishers();
    void createServices();
    void createSubscribers();
    void createRos2Entities(const char *node_name);
    void configWifi(String ssid, String password, IPAddress ip, int port);
    bool initSPIFFS();
    bool createComponent(const char *type, const char *name, JsonObject &component);
    bool createComponentServo(const char *name, JsonObject &component);
    bool createComponentHC_SR04(const char *name, JsonObject &component);

public:
    Configuration(const char *node_name, int spintime_ms, int max_publishers, int max_services, int max_subscribers);
    ~Configuration();

    void spin();

    void setupHW();

    // Method to retrieve a component by name
    template <typename T>
    T *getComponent(const std::string &name)
    {
        if (components.find(name) != components.end())
        {
            return static_cast<T *>(components[name]);
        }
        return nullptr;
    }

    // Method to retrieve a topic by name
    GeneralTopic *getTopic(const std::string &name)
    {
        if (topics.find(name) != topics.end())
        {
            return topics[name];
        }
        return nullptr;
    }

    // Method to add a topic
    void addTopic(const std::string &name, GeneralTopic *topic)
    {
        topics[name] = topic;
    }

    // Method to add a component
    void addComponent(const std::string &name, void *component)
    {
        components[name] = component;
    }
};

#endif
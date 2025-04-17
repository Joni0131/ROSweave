#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <Arduino.h>
#include <micro_ros_platformio.h>

#include "GeneralRosDefinitions.h"
#include "GeneralTopic.h"
#include "MicroRosPublishers.h"
#include "MicroRosServices.h"
#include "MicroRosSubscribers.h"

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

    void createNode(const char *node_name);
    void createSupport();
    void createAllocator();
    void createPublishers();
    void createServices();
    void createSubscribers();
    void createRos2Entities(const char *node_name);

public:
    Configuration(const char *node_name, int spintime_ms, int max_publishers, int max_services, int max_subscribers);
    ~Configuration();

    void spin();

    void setupHW();
};

#endif
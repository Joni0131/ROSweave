#ifndef MICROROSSERVICES_H
#define MICROROSSERVICES_H

#include <Arduino.h>
#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <vector>

#include "GeneralRosDefinitions.h"
#include "GeneralTopic.h"

class MicroRosServices
{
private:
    /* data */
    rclc_executor_t executor_srv;
    int current_services;
    int max_services;
    std::vector<rcl_service_t> services;
    rcl_node_t *node;
    rclc_support_t *support;
    rcl_allocator_t *allocator;
    int spintime_ms;

    void createService(rosidl_service_type_support_t type_support, char *topic_name);
    void createExecutor();
    void addServiceToExecutor(int id, GeneralTopic *topic, rclc_service_callback_t callback);

public:
    MicroRosServices(int max_services, rcl_node_t *node, rclc_support_t *support, rcl_allocator_t *allocator, int spintime_ms);
    ~MicroRosServices();

    int registerService(GeneralTopic *topic, rclc_service_callback_t callback);
    rcl_service_t *getService(int id);
    void spinSrv();

    int getServiceCount();

};

#endif
#ifndef MICROROSSUBSCRIBERS_H
#define MICROROSSUBSCRIBERS_H

#include <Arduino.h>
#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <vector>

#include "GeneralRosDefinitions.h"
#include "GeneralTopic.h"

class MicroRosSubscribers
{
private:
    /* data */
    rclc_executor_t executor_sub;
    int current_subscriber;
    int max_subscribers;
    std::vector<rcl_subscription_t> subscribers;
    rcl_node_t *node;
    rclc_support_t *support;
    rcl_allocator_t *allocator;
    int spintime_ms;

    void createSubscriber(rosidl_message_type_support_t type_support, char *topic_name);
    void createExecutor();
    void addSubscriberToExecutor(int id, GeneralTopic *topic, rclc_subscription_callback_with_context_t callback, rclc_executor_handle_invocation_t executerHandle, void *context);

public:
    MicroRosSubscribers(int max_subscribers, rcl_node_t *node, rclc_support_t *support, rcl_allocator_t *allocator, int spintime_ms); 
    ~MicroRosSubscribers();

    /*returns a pub id that can be used in the callback*/
    int registerSubscriber(GeneralTopic *topic, rclc_executor_handle_invocation_t executerHandle, rclc_subscription_callback_with_context_t callback, void *context);
    rcl_subscription_t *getSubscriber(int id);
    void spinSub();

    int getSubscriberCount();
};

#endif
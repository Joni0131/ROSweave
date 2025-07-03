#ifndef MICROROSPUBLISHERS_H
#define MICROROSPUBLISHERS_H

#include <Arduino.h>
#include <micro_ros_platformio.h>
#include <rmw_microros/rmw_microros.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <vector>
#include <map>

#include "GeneralRosDefinitions.h"
#include "GeneralTopic.h"

class MicroRosPublishers
{
private:
    /* data */
    rclc_executor_t executor_pub;
    int current_publishers;
    int max_publishers;
    std::vector<rcl_publisher_t> publishers;
    std::vector<rcl_timer_t> timers;
    rcl_node_t *node;
    rclc_support_t *support;
    rcl_allocator_t *allocator;
    int spintime_ms;
    const int timeout_ms = 1000;

    void createPublisher(rosidl_message_type_support_t type_support, char *topic_name);

    /// @brief Create a timer
    /// @param time_out Timeout duration in milliseconds
    /// @param timer_callback Callback function to be called on timer expiration
    void createTimer(int time_out, std::function<void(rcl_timer_t *,int64_t)> timer_callback);
    void createExecutor();
    void addPublisherToExecutor(int id);

    // Static wrapper function for timer callbacks
    static void timerCallbackWrapper(rcl_timer_t *timer, int64_t last_call_time);


public:
    MicroRosPublishers(int max_publishers, rcl_node_t *node, rclc_support_t *support, rcl_allocator_t *allocator, int spintime_ms); 
    ~MicroRosPublishers();

    /*returns a pub id that can be used in the callback*/
    int registerPublisher(GeneralTopic *topic, std::function<void(rcl_timer_t *,int64_t)> timer_callback);
    rcl_publisher_t *getPublisher(int id);
    void spinPub();

    int getPublisherCount();
};

#endif
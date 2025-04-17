#include "MicroRosPublishers.h"

// Map to store timer callbacks
std::map<rcl_timer_t *, std::function<void(rcl_timer_t *, int64_t)>> timer_callbacks;

MicroRosPublishers::MicroRosPublishers(int max_publishers, rcl_node_t *node, rclc_support_t *support, rcl_allocator_t *allocator, int spintime_ms)
{
    this->max_publishers = max_publishers;
    this->current_publishers = 0;
    this->node = node;
    this->support = support;
    this->allocator = allocator;
    this->publishers.reserve(max_publishers);
    this->timers.reserve(max_publishers);
    this->spintime_ms = spintime_ms;
    createExecutor();
    // Synchronize time with the agent
    rmw_uros_sync_session(timeout_ms);
}

MicroRosPublishers::~MicroRosPublishers()
{
    this->publishers.clear();
    this->timers.clear();
    timer_callbacks.clear();
    rclc_executor_fini(&(this->executor_pub));
}

void MicroRosPublishers::createExecutor()
{
    RCCHECK(rclc_executor_init(&(this->executor_pub), &(this->support->context), this->max_publishers, this->allocator));
}

void MicroRosPublishers::createPublisher(rosidl_message_type_support_t type_support, char *topic_name)
{
    rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
    rclc_publisher_init_default(&publisher, this->node, &type_support, topic_name);
    this->publishers.push_back(publisher);
}

void MicroRosPublishers::createTimer(int time_out, std::function<void(rcl_timer_t *,int64_t)> timer_callback)
{
    rcl_timer_t timer = rcl_get_zero_initialized_timer();
    RCCHECK(rclc_timer_init_default(&timer, this->support, time_out, MicroRosPublishers::timerCallbackWrapper));

    // Store the callback in the map
    timer_callbacks[&timer] = timer_callback;
    
    this->timers.push_back(timer);
}

int MicroRosPublishers::registerPublisher(GeneralTopic *topic, std::function<void(rcl_timer_t *,int64_t)> timer_callback)
{
    if (this->current_publishers < this->max_publishers)
    {
        createPublisher(topic->getMsgTypeSupport(), topic->getName());
        createTimer(topic->getTimeout(), timer_callback);
        addPublisherToExecutor(this->current_publishers);
        topic->setID(this->current_publishers);
        this->current_publishers++;
        return this->current_publishers - 1;
    }
    else
    {
        return -1;
    }
}

void MicroRosPublishers::addPublisherToExecutor(int id)
{
    RCCHECK(rclc_executor_add_timer(&(this->executor_pub), &(this->timers[id])));
}

rcl_publisher_t *MicroRosPublishers::getPublisher(int id)
{
    return &(this->publishers[id]);
}

void MicroRosPublishers::spinPub()
{
    RCSOFTCHECK(rclc_executor_spin_some(&(this->executor_pub), RCL_MS_TO_NS(this->spintime_ms)));
}

void MicroRosPublishers::timerCallbackWrapper(rcl_timer_t *timer, int64_t last_call_time)
{
    if (timer_callbacks.find(timer) != timer_callbacks.end())
    {
        // Call the stored callback
        timer_callbacks[timer](timer, last_call_time);
    }
}
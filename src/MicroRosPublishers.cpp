#include "MicroRosPublishers.h"

// Map to store timer callbacks
std::map<rcl_timer_t *, std::function<void(rcl_timer_t *, int64_t)>> timer_callbacks;

MicroRosPublishers::MicroRosPublishers(int max_publishers, rcl_node_t *node, rclc_support_t *support, rcl_allocator_t *allocator, int spintime_ms)
{
    Serial.println("[MicroRosPublishers] Constructor called");
    this->max_publishers = max_publishers;
    this->current_publishers = 0;
    this->node = node;
    this->support = support;
    this->allocator = allocator;
    this->publishers.reserve(max_publishers);
    this->timers.reserve(max_publishers);
    this->spintime_ms = spintime_ms;
    Serial.println("[MicroRosPublishers] Creating executor...");
    createExecutor();
    Serial.println("[MicroRosPublishers] Executor created");
    // Synchronize time with the agent
    Serial.println("[MicroRosPublishers] Synchronizing time with agent...");
    rmw_uros_sync_session(timeout_ms);
    Serial.println("[MicroRosPublishers] Time synchronized");
}

MicroRosPublishers::~MicroRosPublishers()
{
    Serial.println("[MicroRosPublishers] Destructor called");
    this->publishers.clear();
    this->timers.clear();
    timer_callbacks.clear();
    rclc_executor_fini(&(this->executor_pub));
    Serial.println("[MicroRosPublishers] Cleaned up");
}

void MicroRosPublishers::createExecutor()
{
    Serial.println("[MicroRosPublishers] Initializing executor...");
    RCCHECK(rclc_executor_init(&(this->executor_pub), &(this->support->context), this->max_publishers, this->allocator));
    Serial.println("[MicroRosPublishers] Executor initialized");
}

void MicroRosPublishers::createPublisher(rosidl_message_type_support_t type_support, char *topic_name)
{
    Serial.printf("[MicroRosPublishers] Creating publisher for topic: %s\n", topic_name);
    rcl_publisher_t publisher = rcl_get_zero_initialized_publisher();
    rclc_publisher_init_default(&publisher, this->node, &type_support, topic_name);
    this->publishers.push_back(publisher);
    Serial.println("[MicroRosPublishers] Publisher created and added to vector");
}

void MicroRosPublishers::createTimer(int time_out, std::function<void(rcl_timer_t *,int64_t)> timer_callback)
{
    Serial.printf("[MicroRosPublishers] Creating timer with timeout: %d\n", time_out);
    rcl_timer_t timer = rcl_get_zero_initialized_timer();
    RCCHECK(rclc_timer_init_default(&timer, this->support, time_out * 1000000ULL, MicroRosPublishers::timerCallbackWrapper));
    Serial.println("[MicroRosPublishers] Timer initialized");
    this->timers.push_back(timer);
    rcl_timer_t* timer_ptr = &this->timers.back();
    timer_callbacks[timer_ptr] = timer_callback;
    Serial.println("[MicroRosPublishers] Timer callback stored in map");
    Serial.println("[MicroRosPublishers] Timer added to vector");
}

int MicroRosPublishers::registerPublisher(GeneralTopic *topic, std::function<void(rcl_timer_t *,int64_t)> timer_callback)
{
    Serial.printf("[MicroRosPublishers] Registering publisher for topic: %s\n", topic->getName());
    if (this->current_publishers < this->max_publishers)
    {
        createPublisher(topic->getMsgTypeSupport(), topic->getName());
        createTimer(topic->getTimeout(), timer_callback);
        addPublisherToExecutor(this->current_publishers);
        topic->setID(this->current_publishers);
        this->current_publishers++;
        Serial.printf("[MicroRosPublishers] Publisher registered with ID: %d\n", this->current_publishers - 1);
        return this->current_publishers - 1;
    }
    else
    {
        Serial.println("[MicroRosPublishers] Max publishers reached, cannot register more");
        return -1;
    }
}

void MicroRosPublishers::addPublisherToExecutor(int id)
{
    Serial.printf("[MicroRosPublishers] Adding timer to executor, id: %d\n", id);
    RCCHECK(rclc_executor_add_timer(&(this->executor_pub), &(this->timers[id])));
    Serial.println("[MicroRosPublishers] Timer added to executor");
}

rcl_publisher_t *MicroRosPublishers::getPublisher(int id)
{
    Serial.printf("[MicroRosPublishers] Getting publisher with id: %d\n", id);
    return &(this->publishers[id]);
}

void MicroRosPublishers::spinPub()
{
    RCSOFTCHECK(rclc_executor_spin_some(&(this->executor_pub), RCL_MS_TO_NS(this->spintime_ms)));
}

int MicroRosPublishers::getPublisherCount()
{
    Serial.printf("[MicroRosPublishers] Current publisher count: %d\n", this->current_publishers);
    return this->current_publishers;
}

void MicroRosPublishers::timerCallbackWrapper(rcl_timer_t *timer, int64_t last_call_time)
{
    Serial.println("[MicroRosPublishers] timerCallbackWrapper called");
    if (timer_callbacks.find(timer) != timer_callbacks.end())
    {
        Serial.println("[MicroRosPublishers] Found timer callback, calling...");
        timer_callbacks[timer](timer, last_call_time);
        Serial.println("[MicroRosPublishers] Timer callback executed");
    }
    else
    {
        Serial.println("[MicroRosPublishers] Timer callback not found in map");
    }
}
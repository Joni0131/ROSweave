#include "MicroRosSubscribers.h"

MicroRosSubscribers::MicroRosSubscribers(int max_subscribers, rcl_node_t *node, rclc_support_t *support, rcl_allocator_t *allocator, int spintime_ms)
{
    Serial.println("[MicroRosSubscribers] Constructor called");
    this->max_subscribers = max_subscribers;
    this->node = node;
    this->support = support;
    this->allocator = allocator;
    this->spintime_ms = spintime_ms;
    this->current_subscriber = 0;
    this->subscribers = std::vector<rcl_subscription_t>(max_subscribers);
    createExecutor();
}

MicroRosSubscribers::~MicroRosSubscribers()
{
    Serial.println("[MicroRosSubscribers] Destructor called");
    RCCHECK(rclc_executor_fini(&executor_sub));
    Serial.println("[MicroRosSubscribers] Executor finalized");
}

void MicroRosSubscribers::createExecutor()
{
    Serial.println("[MicroRosSubscribers] Initializing executor...");
    RCCHECK(rclc_executor_init(&executor_sub, &(this->support->context), this->max_subscribers, this->allocator));
    Serial.println("[MicroRosSubscribers] Executor initialized");
}

void MicroRosSubscribers::addSubscriberToExecutor(int id, GeneralTopic *topic, rclc_subscription_callback_with_context_t callback, rclc_executor_handle_invocation_t executerHandle, void *context)
{
    Serial.printf("[MicroRosSubscribers] Adding subscriber to executor, id: %d, topic: %s\n", id, topic->getName());
    Serial.printf("[MicroRosSubscribers] Callback address: %p, Context: %p\n", (void*)callback, context);
    RCCHECK(rclc_executor_add_subscription_with_context(&executor_sub, getSubscriber(id), topic->getMsg(), callback, context, executerHandle));
    Serial.println("[MicroRosSubscribers] Subscriber added to executor");
}

int MicroRosSubscribers::registerSubscriber(GeneralTopic *topic, rclc_executor_handle_invocation_t executerHandle, rclc_subscription_callback_with_context_t callback, void *context)
{
    Serial.printf("[MicroRosSubscribers] Registering subscriber for topic: %s\n", topic->getName());
    if (current_subscriber >= max_subscribers)
    {
        Serial.println("[MicroRosSubscribers] Max subscribers reached, cannot register more");
        return -1;
    }
    createSubscriber(topic->getMsgTypeSupport(), topic->getName());
    addSubscriberToExecutor(current_subscriber, topic, callback, executerHandle, context);
    topic->setID(current_subscriber);
    current_subscriber++;
    Serial.printf("[MicroRosSubscribers] Subscriber registered with ID: %d\n", current_subscriber - 1);
    return current_subscriber - 1;
}

void MicroRosSubscribers::createSubscriber(rosidl_message_type_support_t type_support, char *topic_name)
{
    Serial.printf("[MicroRosSubscribers] Creating subscriber for topic: %s\n", topic_name);
    rcl_subscription_t sub = rcl_get_zero_initialized_subscription();
    RCCHECK(rclc_subscription_init_default(&sub, node, &type_support, topic_name));
    subscribers.push_back(sub);
    Serial.println("[MicroRosSubscribers] Subscriber created and added to vector");
}

rcl_subscription_t *MicroRosSubscribers::getSubscriber(int id)
{
    Serial.printf("[MicroRosSubscribers] Getting subscriber with id: %d\n", id);
    return &subscribers[id];
}

void MicroRosSubscribers::spinSub()
{
    RCSOFTCHECK(rclc_executor_spin_some(&executor_sub, RCL_MS_TO_NS(this->spintime_ms)));
}

int MicroRosSubscribers::getSubscriberCount()
{
    Serial.printf("[MicroRosSubscribers] Current subscriber count: %d\n", current_subscriber);
    return current_subscriber;
}

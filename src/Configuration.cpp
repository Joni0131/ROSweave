#include "Configuration.h"

Configuration::Configuration(const char *node_name, int spintime_ms, int max_publishers, int max_services, int max_subscribers)
{
    this->node_name = node_name;
    this->spintime_ms = spintime_ms;
    this->max_publishers = max_publishers;
    this->max_services = max_services;
    this->max_subscribers = max_subscribers;

    createRos2Entities(node_name);
}

Configuration::~Configuration()
{
    delete publishers;
    delete services;
    delete subscribers;
}

void Configuration::createRos2Entities(const char *node_name)
{
    createAllocator();
    createSupport();
    createNode(node_name);
    createPublishers();
    createServices();
    createSubscribers();
}

void Configuration::createNode(const char *node_name)
{
    RCCHECK(rclc_node_init_default(&(this->node), node_name, "", &(this->support)));
}

void Configuration::createSupport()
{
    RCCHECK(rclc_support_init(&(this->support), 0, NULL, &(this->allocator)));
}

void Configuration::createAllocator()
{
    this->allocator = rcl_get_default_allocator();
}

void Configuration::createPublishers()
{
    this->publishers = new MicroRosPublishers(this->max_publishers, &(this->node), &(this->support), &(this->allocator), this->spintime_ms);
}

void Configuration::createServices()
{
    this->services = new MicroRosServices(this->max_services, &(this->node), &(this->support), &(this->allocator), this->spintime_ms);
}

void Configuration::createSubscribers()
{
    this->subscribers = new MicroRosSubscribers(this->max_subscribers, &(this->node), &(this->support), &(this->allocator), this->spintime_ms);
}

void Configuration::spin()
{
    this->publishers->spinPub();
    this->services->spinSrv();
    this->subscribers->spinSub();
}

void Configuration::setupHW()
{
    // This function is meant to be overridden by the user
    // It should be used to setup the hardware
    // For example, setting up the motor controller, sensors, etc.
    // This function is called once at the beginning of the program
    // TODO: this is the next step
}
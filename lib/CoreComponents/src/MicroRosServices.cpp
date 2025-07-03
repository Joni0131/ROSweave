#include "MicroRosServices.h"

MicroRosServices::MicroRosServices(int max_services, rcl_node_t *node, rclc_support_t *support, rcl_allocator_t *allocator, int spintime_ms)
{
    this->max_services = max_services;
    this->node = node;
    this->support = support;
    this->allocator = allocator;
    this->spintime_ms = spintime_ms;
    this->current_services = 0;
    this->services = std::vector<rcl_service_t>(max_services);
    this->createExecutor();
}

MicroRosServices::~MicroRosServices()
{
    rclc_executor_fini(&this->executor_srv);
}

void MicroRosServices::createService(rosidl_service_type_support_t type_support, char *topic_name)
{
    this->services[current_services] = rcl_get_zero_initialized_service();
    RCCHECK(rclc_service_init_default(&(this->services[current_services]), this->node, &type_support, topic_name));
}

void MicroRosServices::createExecutor()
{
    RCCHECK(rclc_executor_init(&(this->executor_srv), &(this->support->context), this->max_services, this->allocator));
}

int MicroRosServices::registerService(GeneralTopic *topic, rclc_service_callback_with_context_t callback, void *context)
{
    if (this->current_services < this->max_services)
    {
        this->createService(topic->getSrvTypeSupport(), topic->getName());
        this->addServiceToExecutor(this->current_services, topic, callback, context);
        topic->setID(this->current_services);
        this->current_services++;
        return this->current_services - 1;
    }
    else
    {
        return -1;
    }
}

void MicroRosServices::addServiceToExecutor(int id, GeneralTopic *topic, rclc_service_callback_with_context_t callback, void *context)
{
    RCCHECK(rclc_executor_add_service_with_context(&this->executor_srv, getService(id), topic->getService()->request, topic->getService()->response, callback, context));
}

rcl_service_t *MicroRosServices::getService(int id)
{
    return &this->services[id];
}

void MicroRosServices::spinSrv()
{
    RCSOFTCHECK(rclc_executor_spin_some(&(this->executor_srv), RCL_MS_TO_NS(this->spintime_ms)));
}

int MicroRosServices::getServiceCount()
{
    return this->current_services;
}
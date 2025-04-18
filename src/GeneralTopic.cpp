#include "GeneralTopic.h"
#include <cstring>
#include <cstdlib>

GeneralTopic::GeneralTopic(char *name, size_t name_size, size_t msg_size, int timeout, rosidl_message_type_support_t msg_type_support)
    : timeout(timeout), msg_type_support(msg_type_support), is_service(false)
{
    this->name = (char *)malloc(name_size);
    strncpy(this->name, name, name_size - 1);
    this->name[name_size - 1] = '\0'; // Ensure null termination
    this->msg = malloc(msg_size);
}

GeneralTopic::GeneralTopic(char *name, size_t name_size, size_t req_size, size_t res_size, int timeout, rosidl_service_type_support_t srv_type_support)
    : timeout(timeout), srv_type_support(srv_type_support), is_service(true)
{
    this->name = (char *)malloc(name_size);
    strncpy(this->name, name, name_size - 1);
    this->name[name_size - 1] = '\0'; // Ensure null termination
    this->service.request = malloc(req_size);
    this->service.response = malloc(res_size);
}

GeneralTopic::~GeneralTopic()
{
    free(this->name);
    if (is_service) {
        free(this->service.request);
        free(this->service.response);
    } else {
        free(this->msg);
    }
}

int GeneralTopic::getID()
{
    return this->id;
}

int GeneralTopic::getTimeout()
{
    return this->timeout;
}

char* GeneralTopic::getName()
{
    return this->name;
}

void* GeneralTopic::getMsg()
{
    return this->msg;
}

srv_msg GeneralTopic::getService()
{
    return this->service;
}

rosidl_message_type_support_t GeneralTopic::getMsgTypeSupport()
{
    return this->msg_type_support;
}

rosidl_service_type_support_t GeneralTopic::getSrvTypeSupport()
{
    return this->srv_type_support;
}

bool GeneralTopic::isService()
{
    return this->is_service;
}

void GeneralTopic::setID(int id)
{
    this->id = id;
}

void GeneralTopic::setMsg(void *msg)
{
    this->msg = msg;
}

void GeneralTopic::setService(void *request, void *response)
{
    this->service.request = request;
    this->service.response = response;
}
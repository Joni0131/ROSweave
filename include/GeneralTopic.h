#ifndef GENERALTOPIC_H
#define GENERALTOPIC_H

#include <rosidl_runtime_c/message_type_support_struct.h>
#include <rosidl_runtime_c/service_type_support_struct.h>

typedef struct srv_msg
{
    void *request;
    void *response;
} srv_msg;

class GeneralTopic
{
private:
    char *name;
    void *msg;
    srv_msg service;
    rosidl_message_type_support_t msg_type_support;
    rosidl_service_type_support_t srv_type_support;
    int timeout; // in ns
    int id;
    bool is_service;

public:
    GeneralTopic(char *name, size_t name_size, size_t msg_size, int timeout, rosidl_message_type_support_t msg_type_support);
    GeneralTopic(char *name, size_t name_size, size_t req_size, size_t res_size, int timeout, rosidl_service_type_support_t srv_type_support);
    ~GeneralTopic();

    int getID();
    int getTimeout();
    char* getName();
    void* getMsg();
    srv_msg getService();
    rosidl_message_type_support_t getMsgTypeSupport();
    rosidl_service_type_support_t getSrvTypeSupport();
    bool isService();

    void setID(int id);
    void setService(void *request, void *response);
};

#endif // GENERALTOPIC_H
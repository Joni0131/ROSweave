#include "ServiceTest.h"

ServiceTest::ServiceTest(GeneralTopic *serviceTopic, MicroRosServices *services)
{
    this->serviceTopic = serviceTopic;
    this->services = services;

    this->initTopics();
    this->registerCallbacks();
}

ServiceTest::~ServiceTest()
{
}

void ServiceTest::initTopics(){
    srv_msg *serviceTopicMsg = (srv_msg *)this->serviceTopic->getService();
    ((custom_interfaces__srv__ServiceTest_Request *)serviceTopicMsg->request)->a = 0;
    ((custom_interfaces__srv__ServiceTest_Request *)serviceTopicMsg->request)->b = 0;
    ((custom_interfaces__srv__ServiceTest_Response *)serviceTopicMsg->response)->sum = 0;
}

void ServiceTest::registerCallbacks()
{
    Serial.println("[ServiceTest] Registering service callback");
    this->services->registerService(this->serviceTopic, SERVICETEST_targetCallbackWrapper, this);
    Serial.println("[ServiceTest] Service callback registered");
}

void ServiceTest::serviceCallback(const void * request, const void * responds)
{
    Serial.println("[ServiceTest] serviceCallback called");
    custom_interfaces__srv__ServiceTest_Request *req = (custom_interfaces__srv__ServiceTest_Request *)request;
    custom_interfaces__srv__ServiceTest_Response *res = (custom_interfaces__srv__ServiceTest_Response *)responds;

    Serial.printf("[ServiceTest] Received request: a=%" PRId64 ", b=%" PRId64 "\n", req->a, req->b);
    res->sum = req->a + req->b;
    Serial.printf("[ServiceTest] Sending response: sum=%" PRId64 "\n", res->sum);
}

void SERVICETEST_targetCallbackWrapper(const void *request, void *responds, void *serviceTest)
{
    ((ServiceTest *)serviceTest)->serviceCallback(request, responds);
}

void Configuration::setupServiceTest()
{
    Serial.println("[Configuration] Setting up ServiceTest component...");

    // Create topic names
    std::string serviceTopicName = "service_test";
    
    // Initialize the service topic
    GeneralTopic *serviceTopic = new GeneralTopic(
        const_cast<char *>(serviceTopicName.c_str()), serviceTopicName.size() + 1, sizeof(custom_interfaces__srv__ServiceTest_Request), sizeof(custom_interfaces__srv__ServiceTest_Response), 1000, *rosidl_typesupport_c__get_service_type_support_handle__custom_interfaces__srv__ServiceTest());

    // Add the service topic to the topics map
    addTopic(serviceTopicName.c_str(), serviceTopic);
    Serial.println("[Configuration] Service topic added to map.");
    
    // Create and initialize the ServiceTest component
    ServiceTest *serviceTest = new ServiceTest(serviceTopic, this->services);
    Serial.printf("[Configuration] ServiceTest '%s' initialized.\n", serviceTopicName.c_str());

    // Store the instance in the components map
    components[serviceTopicName] = serviceTest;
    Serial.printf("[Configuration] ServiceTest '%s' stored in components map.\n", serviceTopicName.c_str());

}
#include "Configuration.h"

Configuration::Configuration(const char *node_name, int spintime_ms, int max_publishers, int max_services, int max_subscribers)
{
    this->node_name = node_name;
    this->spintime_ms = spintime_ms;
    this->max_publishers = max_publishers;
    this->max_services = max_services;
    this->max_subscribers = max_subscribers;

    //FIXME: TODO: The problem why the esp crashes on the creation of the Configuration class is that the transport layer is not set before. Change the creation of the allocator and all subsequent ros nodes to be executed after the Hardware setup so the Wifi is configured correctly. Maybe restructure the HW setup because of possible dependencies.

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
    Serial.println("Creating ROS2 entities...");
    createAllocator();
    Serial.println("Allocator created.");
    createSupport();
    Serial.println("Support created.");
    createNode(node_name);
    Serial.println("Node created.");
    createPublishers();
    Serial.println("Publishers created.");
    createServices();
    Serial.println("Services created.");
    createSubscribers();
    Serial.println("Subscribers created.");
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

void Configuration::configWifi(char *ssid, char *password, IPAddress ip, uint16_t port)
{
    set_microros_wifi_transports(ssid, password, ip, port);
    delay(2000);
}

bool Configuration::initSPIFFS()
{
    if (!SPIFFS.begin())
    {
        Serial.println("Failed to start SPIFFS.");
        return false;
    }
    else
    {
        Serial.println("SPIFFS started.");
        // Check if the server folder exists
        if (!SPIFFS.exists("/config.json"))
        {
            Serial.println("Config not found.");
            return false; 
        }
        else
        {
            Serial.println("Config found.");
            return true;
        }
    }
}

void Configuration::setupHW()
{
    // Prepare the file system
    if (!initSPIFFS())
    {
        Serial.println("Failed to initialize SPIFFS.");
        return;
    }

    // Initilize the json document
    JsonDocument doc;
    File configFile = SPIFFS.open("/config.json", "r");
    if (!configFile)
    {
        Serial.println("Failed to open config file.");
        return;
    }
    size_t size = configFile.size();
    char *buffer = new char[size + 1];
    configFile.readBytes(buffer, size);
    buffer[size] = '\0'; // Null-terminate the string
    configFile.close();
    DeserializationError error = deserializeJson(doc, buffer);
    delete[] buffer; // Free the buffer
    if (error)
    {
        Serial.println("Failed to parse config file.");
        return;
    }

    // Extract the wifi configuration
    const char *ssid = doc["wifi"]["ssid"];
    const char *password = doc["wifi"]["password"];

    // Extract the IP address from text to IPAddress
    const char *ip_str = doc["wifi"]["ip"];
    IPAddress ip;
    if (sscanf(ip_str, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) != 4)
    {
        Serial.println("Failed to parse IP address.");
        return;
    }

    // Extract the port convert string to uint16_t
    const char *port_str = doc["wifi"]["port"];
    uint16_t port = atoi(port_str);
    if (port == 0)
    {
        Serial.println("Failed to parse port.");
        return;
    }
    configWifi((char *)ssid, (char *)password, ip, port);
    Serial.println("Wifi configured.");

    // Iterate over each component
    for (int i = 0; i < doc["components"].size(); i++)
    {
        JsonObject component = doc["components"][i];
        const char *type = component["type"];
        const char *name = component["name"];
        Serial.printf("Initializing Component %d: %s (%s)\n", i, name, type);

        // Use a factory function to create components
        if (!createComponent(type, name, component))
        {
            Serial.printf("Failed to initialize component: %s (%s)\n", name, type);
        }
    }

    Serial.println("Components configured.");
}

bool Configuration::createComponent(const char *type, const char *name, JsonObject &component)
{
    if (strcmp(type, "Servo") == 0)
    {
        return createComponentServo(name, component);
    }
    else if (strcmp(type, "IMU") == 0)
    {
        return false; // createComponentIMU(name, component);
    }
    else
    {
        Serial.printf("Unknown component type: %s\n", type);
        return false;
    }
}

bool Configuration::createComponentServo(const char *name, JsonObject &component)
{
    std::string targetTopicName = std::string(name) + "_target";
    std::string infoTopicName = std::string(name) + "_info";

    // Extract the update interval
    int updateInterval = atoi(component["config"]["topicUpdateInterval"]);

    // Initialize topics
    GeneralTopic *targetTopic = new GeneralTopic(
        const_cast<char *>(targetTopicName.c_str()), targetTopicName.size() + 1, sizeof(custom_interfaces__msg__ServoMotor), updateInterval, *rosidl_typesupport_c__get_message_type_support_handle__custom_interfaces__msg__ServoMotor());
    GeneralTopic *infoTopic = new GeneralTopic(
        const_cast<char *>(infoTopicName.c_str()), infoTopicName.size() + 1, sizeof(custom_interfaces__msg__ServoMotor), updateInterval, *rosidl_typesupport_c__get_message_type_support_handle__custom_interfaces__msg__ServoMotor());

    // Add topics to the topics map
    addTopic(targetTopicName.c_str(), targetTopic);
    addTopic(infoTopicName.c_str(), infoTopic);

    // Get the motor type
    const char *motorTypeStr = component["config"]["type"];
    ServoMotorType motorType;
    if (strcmp(motorTypeStr, "SG90_180") == 0)
    {
        motorType = SG90_180;
    }
    else if (strcmp(motorTypeStr, "SG90_360") == 0)
    {
        motorType = SG90_360;
    }
    else
    {
        Serial.printf("Unknown motor type: %s\n", motorTypeStr);
        return false;
    }

    // Extract pin configuration
    JsonArray pinArray = component["config"]["pins"];
    std::vector<PinConfiguration> pin_config;
    for (int i = 0; i < pinArray.size(); i++)
    {
        PinConfiguration pin;
        pin.pin = pinArray[i]["pin"];
        pin.mode = 0;
        pin.value = 0;
        pin_config.push_back(pin);
    }

    // Create and initialize the ServoMotor
    ServoMotor *servo = new ServoMotor(&pin_config, targetTopic, infoTopic, this->publishers, this->subscribers, this->services, motorType);
    Serial.printf("ServoMotor '%s' initialized.\n", name);

    // Store the instance in the components map
    components[name] = servo;

    return true;
}
#include "Configuration.h"

Configuration::Configuration(const char *node_name, int spintime_ms, int max_publishers, int max_services, int max_subscribers)
{
    this->node_name = node_name;
    this->spintime_ms = spintime_ms;
    this->max_publishers = max_publishers;
    this->max_services = max_services;
    this->max_subscribers = max_subscribers;
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
    delay(this->spintime_ms); // Delay to allow other tasks to run
}

void Configuration::configWifi(String ssid, String password, IPAddress ip, int port)
{
    set_microros_wifi_transports(ssid.begin(), password.begin(), ip, port);
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
    }else{
        Serial.println("Initilized SPIFFS");
    }

    // Initilize the json document
    JsonDocument doc;
    File configFile = SPIFFS.open("/config.json", "r");
    if (!configFile)
    {
        Serial.println("Failed to open config file.");
        return;
    }else{
        Serial.println("Opened config file.");
    }
    size_t size = configFile.size();
    char *buffer = new char[size + 1];
    Serial.println("Reading bytes...");
    configFile.readBytes(buffer, size);
    buffer[size] = '\0'; // Null-terminate the string
    configFile.close();
    Serial.println("Closed File");
    DeserializationError error = deserializeJson(doc, buffer);
    Serial.println("Deserialized Json.");
    delete[] buffer; // Free the buffer
    if (error)
    {
        Serial.println("Failed to parse config file.");
        return;
    }

    // Extract the wifi configuration
    String ssid = doc["wifi"]["ssid"];
    String password = doc["wifi"]["password"];

    // Extract the IP address from text to IPAddress
    const char *ip_str = doc["wifi"]["ip"];
    Serial.print("Parsed ssid:");
    Serial.println(ssid);
    Serial.print("Parsed password:");
    Serial.println(password);
    Serial.print("Parsed ip_str:");
    Serial.println(ip_str);
    IPAddress ip;
    if (sscanf(ip_str, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) != 4)
    {
        Serial.println("Failed to parse IP address.");
        return;
    }

    // Extract the port convert string to int
    const char *port_str = doc["wifi"]["port"];
    Serial.print("Parsed port_str:");
    Serial.println(port_str);
    int port = atoi(port_str);
    if (port == 0)
    {
        Serial.println("Failed to parse port.");
        return;
    }
    configWifi(ssid, password, ip, port); // Check order with old project again seems it is stuck here
    Serial.println("Wifi configured.");

    // Create ROS2 entities
    createRos2Entities(this->node_name);
    Serial.println("ROS2 entities created.");

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
    ServoMotor::ServoMotorFactoryResult servoResult = ServoMotor::createFromJsonServo(name, component, this->publishers, this->subscribers, this->services);
    if (servoResult.servo == nullptr || servoResult.targetTopic == nullptr || servoResult.infoTopic == nullptr)
    {
        Serial.printf("[createComponentServo] Failed to create ServoMotor '%s'.\n", name);
        return false;
    }

    this->addComponent(name, servoResult.servo);
    this->addTopic(servoResult.targetTopic->getName(), servoResult.targetTopic);
    this->addTopic(servoResult.infoTopic->getName(), servoResult.infoTopic);
    return true;
}
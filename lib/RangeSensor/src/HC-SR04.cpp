#include "HC-SR04.h"

HC_SR04::HC_SR04(std::vector<PinConfiguration> *pin_config, GeneralTopic *infoTopic, MicroRosPublishers *publishers)
{
    this->infoTopic = infoTopic;
    this->publishers = publishers;

    // Initialize the sonar sensor with the pin configuration
    if (pin_config->size() < 2 || (*pin_config)[0].pin < 0 || (*pin_config)[1].pin < 0)
    {
        Serial.println("Pin configuration for HC-SR04 is invalid. At least 2 pins are required.");
        return;
    }

    if ((*pin_config)[0].mode != OUTPUT || (*pin_config)[1].mode != INPUT)
    {
        Serial.println("Pin modes for HC-SR04 are invalid. Trigger pin must be OUTPUT and Echo pin must be INPUT.");
        return;
    }

    if ((*pin_config)[0].pin == (*pin_config)[1].pin)
    {
        Serial.println("Pin configuration for HC-SR04 is invalid. Trigger pin and Echo pin must be different.");
        return;
    }

    // Create the sonar object with the trigger and echo pins
    this->sonar = new NewPing((*pin_config)[0].pin, (*pin_config)[1].pin, 200);

    // Initialize topics
    initTopics();
    // Register callbacks for the info topic
    registerCallbacks();
}

HC_SR04::~HC_SR04()
{
    if (sonar)
    {
        delete sonar;
        sonar = nullptr;
    }
}

void HC_SR04::initTopics()
{
    sensor_msgs__msg__Range *range_msg = (sensor_msgs__msg__Range *)this->infoTopic.getMsg();

    range_msg->radiation_type = 0;           // Assuming 0 for ultrasonic
    range_msg->field_of_view = 0.5235987756; // Is 30° for the HC-SR04 => 30 degrees in radians
    range_msg->min_range = 0.002;            // Minimum range in meters
    range_msg->max_range = 0.3;              // Maximum range in meters
    range_msg->variance = 0.0;               // Assuming no variance for simplicity
    range_msg->range = 0.0;                  // Initial range value

    // Set the header information
    range_msg->header.frame_id.data = (char *)"hc_sr04_frame";
    range_msg->header.frame_id.size = strlen(range_msg->header.frame_id.data);
    range_msg->header.stamp.sec = 0;
    range_msg->header.stamp.nanosec = 0;
}

void HC_SR04::infoCallback(rcl_timer_t *timer, int64_t last_call_time)
{
    // Get corrected epoch time
    int64_t time_ns = rmw_uros_epoch_nanos();
    sensor_msgs__msg__Range *msg = (sensor_msgs__msg__Range *)this->infoTopic->getMsg();
    msg->header.stamp.sec = (int32_t)(time_ns / 1000000000);
    msg->header.stamp.nanosec = (uint32_t)(time_ns % 1000000000);

    // Perform a ping and get the distance in centimeters
    unsigned long distance_cm = sonar->ping_cm();

    msg->range = distance_cm / 100.0; // Convert cm to meters
    // No need to call setMsg if getMsg() returns the buffer used for publishing
    RCSOFTCHECK(rcl_publish(this->publishers->getPublisher(this->infoTopic->getID()), msg, NULL));
}

void HC_SR04::registerCallbacks()
{
    Serial.println("[HC_SR04] Registering publisher and subscriber callbacks");
    this->publishers->registerPublisher(this->infoTopic, [this](rcl_timer_t *timer, uint64_t currentTime)
                                        { this->infoCallback(timer, currentTime); });
}

HC_SR04 *HC_SR04::createFromJsonHC_SR04(
    const char *name,
    JsonObject &component,
    MicroRosPublishers *publishers)
{
}
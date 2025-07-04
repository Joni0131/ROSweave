#ifndef HC_SR04_CPP
#define HC_SR04_CPP

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <string>
#include <vector>

#include "NewPing.h"
#include "GeneralTopic.h"
#include "PinConfiguration.h"
#include "MicroRosPublishers.h"
#include "MicroRosSubscribers.h"
#include "MicroRosServices.h"

#include "sensor_msgs/msg/range.h"

class HC_SR04
{
private:
    /* data */
    NewPing *sonar;
    GeneralTopic *infoTopic;
    MicroRosPublishers *publishers;

    void initTopics();
    void registerCallbacks();

public:
    HC_SR04(std::vector<PinConfiguration> *pin_config, GeneralTopic *infoTopic, MicroRosPublishers *publishers);
    ~HC_SR04();

    void infoCallback(rcl_timer_t *timer, int64_t last_call_time);

    static HC_SR04 *createFromJsonHC_SR04(
        const char *name,
        JsonObject &component,
        MicroRosPublishers *publishers
    );
};


#endif // HC_SR04_CPP
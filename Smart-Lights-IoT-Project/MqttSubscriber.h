//
// Created by Catalin on 5/24/2021.
//

#ifndef SMART_LIGHTS_IOT_PROJECT_MQTTSUBSCRIBER_H
#define SMART_LIGHTS_IOT_PROJECT_MQTTSUBSCRIBER_H
#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include <vector>
#include <functional>
#include "mqtt/client.h"
#include "nlohmann/json.hpp"
#include "SmartLightController.h"

class MqttSubscriber {
public :
    explicit MqttSubscriber(SmartLightController *rawController): controller(rawController){}
    void start();

private:
    void subscribe();
    const std::string BROKER_ADDRESS = "tcp://localhost:1883";
    const std::string CLIENT_ID= "microphone";
    const std::string INPUT_TOPIC_NAME= "topic-input";
    const std::string OUTPUT_TOPIC_NAME= "topic-output";

    std::shared_ptr<SmartLightController> controller;

};


#endif //SMART_LIGHTS_IOT_PROJECT_MQTTSUBSCRIBER_H

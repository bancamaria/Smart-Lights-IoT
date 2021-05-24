//
// Created by Catalin on 5/24/2021.
//

#include "MqttSubscriber.h"
using namespace nlohmann;

constexpr int QOS_0 = 0;
void MqttSubscriber::start() {
    std::thread subThread(&MqttSubscriber::subscribe,this);
    subThread.detach();
}
void MqttSubscriber::subscribe() {
    std::cout << "Started listening for messages to address " << BROKER_ADDRESS;
    mqtt::client client(BROKER_ADDRESS, CLIENT_ID, mqtt::create_options(5));

        mqtt::connect_options opts;
    opts.set_automatic_reconnect(5, 30);
    opts.set_clean_session(false);
    opts.set_mqtt_version(MQTTVERSION_5);

    try{

        std::cout << "Connecting to the MQTT server..." << std::flush;
        client.connect(opts);
        std::cout << "Connected!" << std::endl;
        std::cout << "Subscribing to topic: " << INPUT_TOPIC_NAME << std::endl;

        mqtt::subscribe_options subOpts;
        mqtt::properties props1 {
                { mqtt::property::SUBSCRIPTION_IDENTIFIER, 1},
        };
        client.subscribe(INPUT_TOPIC_NAME, QOS_0, subOpts, props1);
        std::cout << "Subscribed to topic: " << INPUT_TOPIC_NAME << std::endl;
        while (true) {
//        BLock until a new message arrives.
            auto mqttMessage = client.consume_message();
            //        Returns a shared pointer to the message payload
            if (mqttMessage) {

                try {
                    json requestJson = json::parse(mqttMessage->to_string());
                    if (requestJson.contains("disconnect")) {
                        if (requestJson["disconnect"])
                            break;
                    } else if (requestJson.contains("record")) {
                        //                There has been a new sound recorded, so act as
                        json response = controller->onSoundRecordedMqtt(requestJson["record"].get<std::string>());
                        auto pubmsg = mqtt::make_message(OUTPUT_TOPIC_NAME, response.dump());
                        pubmsg->set_qos(QOS_0);
                        client.publish(pubmsg);

                    } else if (requestJson.contains("brightness")) {
                        if (requestJson.contains("presence")) {
                            int brightness = requestJson["brightness"].get<int>();
                            int presence = requestJson["presence"].get<int>();
                            json resp = controller->onBrightnessRecordedMqtt(brightness, presence == 1);
                            auto pubmsg = mqtt::make_message(OUTPUT_TOPIC_NAME, resp.dump());
                            pubmsg->set_qos(QOS_0);
                            client.publish(pubmsg);
                        }
                    } else {
                        auto pubmsg = mqtt::make_message(OUTPUT_TOPIC_NAME, "INVALID REQUEST");
                        pubmsg->set_qos(QOS_0);
                        client.publish(pubmsg);
                    }
                }catch( nlohmann::json::exception &err){
                    std::cerr << err.what() << std::endl;
                    auto pubmsg = mqtt::make_message(OUTPUT_TOPIC_NAME, err.what());
                    pubmsg->set_qos(QOS_0);
                    client.publish(pubmsg);
                }
            } else if (!client.is_connected()) {
                std::cout << "Lost connection" << std::endl;
                while (!client.is_connected()) {
                    std::this_thread::sleep_for(chrono::milliseconds(200));
                }
                std::cout << "Re-established connection" << std::endl;
            }
        }
        std::cout << "Disconnecting from MQTT server..." << std::flush;
        client.disconnect();
        std::cout << "Disconnected from MQTT server" << std::endl;
    }catch (const mqtt::exception &exc) {
        std::cerr << exc.what() << " [" << exc.get_reason_code() << "]" << std::endl;
    }

}

//
// Created by Catalin on 3/13/2021.
//

#ifndef SMART_LIGHTS_IOT_PROJECT_SMARTLAMP_H
#define SMART_LIGHTS_IOT_PROJECT_SMARTLAMP_H

#include "pistache/http.h"
#include "nlohmann/json.hpp"

using namespace Pistache;
using namespace std;
using namespace nlohmann;

namespace smartlamp{

    enum ACTION{
        TURN_ON_LIGHT,
        TURN_OFF_LIGHT,
        CHANGE_COLOR,
        START_COLOR_PATTERN,
        TURN_ON_BUZZER,
        TURN_OFF_BUZZER,
        CHANGE_INTENSITY
    };

    enum MIC_CONFIG{
        SENSITIVITY,
        PATTERNS
    };

    enum BULB_CONFIG {
        BULB_STATUS,
        INTENSITY
    };

    struct ParametrizedAction{
        ACTION actionType;
        std::string actionParam;
    };

    enum BUZZER_CONFIG{ // the status is going to tell if the alarm is on or not
        BUZZER_STATUS,
    };

    enum COLOURS {
        COLOUR0, COLOUR1, COLOUR2, COLOUR3, COLOUR4
    };


    void to_json(json& j, const ParametrizedAction& p);
    void from_json(const json& j, ParametrizedAction& p);
    extern const std::string EMPTY_PARAM;

    namespace light{
        extern const std::string NONE_COLOR_PATTERN;
        extern const std::string DEFAULT_COLOR;
        extern const int DEFAULT_INTENSITY;
        extern const int MIN_INTENSITY;
        extern const int MAX_INTENSITY;

        struct LightState{
            int intensity = DEFAULT_INTENSITY ;
            std::string colorPattern = NONE_COLOR_PATTERN;
            std::string color = DEFAULT_COLOR;
            bool isOn = false;
        };
        void to_json(json& j, const LightState& p);
        void from_json(const json& j, LightState& p);
    }

};


class SmartLamp {

public:


    explicit SmartLamp(){
        possibleActions.insert(std::make_pair("TURN_ON_LIGHT",smartlamp::ACTION::TURN_ON_LIGHT));
        possibleActions.insert(std::make_pair("TURN_OFF_LIGHT",smartlamp::ACTION::TURN_OFF_LIGHT));
        possibleActions.insert(std::make_pair("CHANGE_COLOR",smartlamp::ACTION::CHANGE_COLOR));
        possibleActions.insert(std::make_pair("START_COLOR_PATTERN",smartlamp::ACTION::START_COLOR_PATTERN));

        possibleActions.insert(std::make_pair("TURN_ON_BUZZER",smartlamp::ACTION::TURN_ON_LIGHT));
        possibleActions.insert(std::make_pair("TURN_OFF_BUZZER",smartlamp::ACTION::TURN_OFF_LIGHT));

        possibleActions.insert(std::make_pair("CHANGE_INTENSITY", smartlamp::ACTION::CHANGE_INTENSITY));
    };

    bool hasMapping(const std::string &mapping);
    void setMicSensitivity(const int &sensititvity);
    int getMicSensitivity();

    std::unordered_map<std::string,smartlamp::ParametrizedAction> getSoundPatterns();
    bool addSoundPattern(const std::string &regexPattern, smartlamp::ACTION action);
    bool addSoundPattern(const std::string &regexPattern, const string& action);
    bool addSoundPattern(const std::string &regexPattern, const string& action, const string&optionValue);

    void setBuzzerStatus(const int &status);
    int getBuzzerStatus();

    void setBulbStatus(const int &status);
    int getBulbStatus();
    void setBulbIntensity(const int &lightValue);
    int getBulbIntensity();

    smartlamp::light::LightState onSoundRecorded(const std::string &soundPattern);


private:
    /*Members that can adjust the microphone */
    /*
     * All the recorded sound patterns that, when detected, will result in a state change of the lamp
     * where the key is the pattern, e.g. '1000101011' and the value is the possible ACTION. */
    std::unordered_map<std::string, smartlamp::ParametrizedAction> soundPatternsMapping;
    std::unordered_map<std::string, smartlamp::ACTION> possibleActions;
    smartlamp::light::LightState currentState;
    int micSensitivity;
    int buzzerStatus;
    int bulbStatus;
    int lightIntensity;
    int brightness;
    int fadeAmount;
};


#endif //SMART_LIGHTS_IOT_PROJECT_SMARTLAMP_H

//
// Created by Catalin on 3/13/2021.
//

#include "SmartLamp.h"
namespace smartlamp{

    void to_json(json& j, const ParametrizedAction& p) {
        j = json{{"actionType", p.actionType}, {"actionParam", p.actionParam}};
    }

    void from_json(const json& j, ParametrizedAction& p) {
        j.at("actionType").get_to(p.actionType);
        j.at("actionParam").get_to(p.actionParam);
    }
    const std::string EMPTY_PARAM = "";

    namespace light{

        void to_json(json& j, const LightState& p) {
            j = json{{"intensity", p.intensity},
                     {"color", p.color},
                     {"colorPattern",p.colorPattern}};
        }

        void from_json(const json& j, LightState& p) {
            j.at("intensity").get_to(p.intensity);
            j.at("colorPattern").get_to(p.colorPattern);
            j.at("color").get_to(p.color);
        }
        const std::string NONE_COLOR_PATTERN = "NONE";
    }
};

using namespace smartlamp;



bool SmartLamp::addSoundPattern(const string &regexPattern, ACTION action) {
    ParametrizedAction parametrizedAction;
    parametrizedAction.actionType = action;
    parametrizedAction.actionParam = EMPTY_PARAM;
    return soundPatternsMapping.insert(std::make_pair(regexPattern, parametrizedAction)).second;
}

bool SmartLamp::addSoundPattern(const string &regexPattern, const string &action) {
    ParametrizedAction parametrizedAction;
    parametrizedAction.actionType = possibleActions[action];
    parametrizedAction.actionParam = EMPTY_PARAM;
    auto result = soundPatternsMapping.insert(std::make_pair(regexPattern, parametrizedAction));
    return result.second;
}


int SmartLamp::getMicSensitivity() {
    return micSensitivity;
}

void SmartLamp::setMicSensitivity(const int &sensitivity) {
    micSensitivity = sensitivity;
}

bool SmartLamp::hasMapping(const string &mapping) {
    return possibleActions.find(mapping) != possibleActions.end();
}

/*
 * To be used whenever there is a newPattern mapping to CHANGE_COLOR or START_LIGHT_PATTERN
 * In the CHANGE_COLOR case, the optionValue parameter is the color's name
 * In the START_LIGHT_PATTERN case, the optionValue parameter is the light pattern */
bool SmartLamp::addSoundPattern(const string &regexPattern, const string &action, const string & optionValue) {
    ParametrizedAction actualAction;
    actualAction.actionType = possibleActions[action];
    actualAction.actionParam = optionValue;
    return soundPatternsMapping.insert(std::make_pair(regexPattern, actualAction)).second;
}

unordered_map<std::string, smartlamp::ParametrizedAction> SmartLamp::getSoundPatterns() {
    return soundPatternsMapping;
}

light::LightState SmartLamp::onSoundRecorded(const string &soundPattern) {

}








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
                     {"colorPattern",p.colorPattern},
                     {"status",p.isOn}};
        }

        void from_json(const json& j, LightState& p) {
            j.at("intensity").get_to(p.intensity);
            j.at("colorPattern").get_to(p.colorPattern);
            j.at("color").get_to(p.color);
            j.at("status").get_to(p.isOn);

        }
        const std::string NONE_COLOR_PATTERN = "NONE";
        const std::string DEFAULT_COLOR = "WHITE";
        const int DEFAULT_INTENSITY = 5;
        const int MIN_INTENSITY = 1;
        const int MAX_INTENSITY = 10;

    }

    namespace buzzer{
        void to_json(json &j, const buzzer::BuzzerState &p) {
            j = json{{"status", p.status},
                     {"snooze_time", p.snooze_time}};
        }

        void from_json(const json &j, buzzer::BuzzerState &p) {
            j.at("status").get_to(p.status);
            j.at("snooze_time").get_to(p.snooze_time);
        }
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

pair<light::LightState, buzzer::BuzzerState> SmartLamp::onSoundRecorded(const string &soundPattern) {
    auto it = soundPatternsMapping.find(soundPattern);
    /*
     * If the soundPattern is not known, we must ignore it, since it is just 'noise' so no new action should be taken
     * */
    if( it == soundPatternsMapping.end())
        return make_pair(currentLightState, currentBuzzerState);

    switch (it->second.actionType) {
        case ACTION::TURN_ON_LIGHT: {
            currentLightState.isOn = true;
            currentLightState.color = light::DEFAULT_COLOR;
            currentLightState.intensity = 5;
            currentLightState.colorPattern = light::NONE_COLOR_PATTERN;
            break;
        }
        case ACTION::TURN_OFF_LIGHT: {
            currentLightState.isOn = false;
            break;
        }
        case ACTION::CHANGE_COLOR: {
            currentLightState.isOn = true;
            currentLightState.colorPattern = light::NONE_COLOR_PATTERN;
            currentLightState.color = it->second.actionParam;
            break;
        }

        case ACTION::START_COLOR_PATTERN: {
            currentLightState.isOn = true;
            currentLightState.color = light::DEFAULT_COLOR;
            currentLightState.colorPattern = it->second.actionParam;
            break;
        }

        case ACTION::TURN_ON_BUZZER: {
            currentBuzzerState.status = true;

            struct tm when = {0};

            when.tm_hour = 7;
            when.tm_min = 00;
            when.tm_sec = 00;

            time_t converted;
            converted = mktime(&when);

            currentBuzzerState.snooze_time = converted;

            break;
        }

        case ACTION::TURN_OFF_BUZZER: {
            currentBuzzerState.status = false;
            break;
        }
    }
        return make_pair(currentLightState, currentBuzzerState); // what we should do here ?
}

void SmartLamp::setBuzzerStatus(const int &status) {
    buzzerStatus = status;
}

void SmartLamp::setBuzzerSnoozeTime(const time_t &snooze_time) {
    buzzerSnoozeTime = snooze_time;
}


time_t SmartLamp::getBuzzerSnoozeTime() {
    return buzzerSnoozeTime;
}


int SmartLamp::getBuzzerStatus() {
    return buzzerStatus;
}

void SmartLamp::setBulbStatus(const int &status) {
    bulbStatus =  status;
}

int SmartLamp::getBulbStatus() {
    return bulbStatus;
}

void SmartLamp::setBulbIntensity(const int &lightValue) {
    lightIntensity = lightValue;
}

int SmartLamp::getBulbIntensity() {
    return lightIntensity;
}

int SmartLamp::getColor() {
    return bulbColor;
}

void SmartLamp::setColor(const int &color) {
    bulbColor = color;
}









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
                     {"isOn",p.isOn}};
        }

        void from_json(const json& j, LightState& p) {
            j.at("intensity").get_to(p.intensity);
            j.at("colorPattern").get_to(p.colorPattern);
            j.at("color").get_to(p.color);
            j.at("isOn").get_to(p.isOn);

        }
        const std::string NONE_COLOR_PATTERN = "NONE";
        const std::string DEFAULT_COLOR = "WHITE";
        const int DEFAULT_INTENSITY = 5;
        const int MIN_INTENSITY = 1;
        const int MAX_INTENSITY = 10;

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
    auto it = soundPatternsMapping.find(soundPattern);
    /*
     * If the soundPattern is not known, we must ignore it, since it is just 'noise' so no new action should be taken
     * */
    if( it == soundPatternsMapping.end())
        return currentState;

    switch (it->second.actionType) {
        case ACTION::TURN_ON_LIGHT: {
            currentState.isOn = true;
            currentState.color = light::DEFAULT_COLOR;
            currentState.intensity = 5;
            currentState.colorPattern = light::NONE_COLOR_PATTERN;
            break;
        }
        case ACTION::TURN_OFF_LIGHT: {
            currentState.isOn = false;
            break;
        }
        case ACTION::CHANGE_COLOR: {
            currentState.isOn = true;
            currentState.colorPattern = light::NONE_COLOR_PATTERN;
            currentState.color = it->second.actionParam;
            break;
        }

        case ACTION::START_COLOR_PATTERN: {
            currentState.isOn = true;
            currentState.color = light::DEFAULT_COLOR;
            currentState.colorPattern = it->second.actionParam;
            break;
        }
    }
        return currentState;
}

void SmartLamp::setBuzzerStatus(const int &status) {
    buzzerStatus = status;
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

    if (brightness <= 0 || brightness >= 100)
        fadeAmount = -fadeAmount;

    switch (brightness) {
        case morning:
            if(brightness >= 5 && brightness <= 30)
                while (lightsOn) 	// increase intensity gradually
                    COLOUR = COLOUR1; 	// dim white lights
            brightness += fadeAmount;
            break;
        case afternoon:
            if(brightness >= 60 && brightness <= 100)
                while (lightsOn) 	// increase intensity gradually
                    COLOUR = COLOUR2; 	// almost no white lights
            brightness += fadeAmount;
            break;
        case evening:
            if(brightness >= 30 && brightness <= 60)
                while (lightsOn) 	// increase intensity gradually
                    COLOUR = COLOUR3; 	// dim yellow lights
            brightness += fadeAmount;
            break;
        case night:
            if(brightness >= 0 && brightness <= 5)
                while (lightsOn) 	// increase intensity gradually
                    COLOUR = COLOUR4; 	// colour-changing
            brightness += fadeAmount;
            break;
        default: COLOUR = COLOUR0; // strong whie lights
    }
}

int SmartLamp::getBulbIntensity() {
    return lightIntensity;
}



